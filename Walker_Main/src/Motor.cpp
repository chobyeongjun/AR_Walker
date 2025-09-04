#include "Motor.h"
#include "CAN.h"

#include "MotorData.h"    
#include "JointData.h"    
#include "ErrorManager.h"
#include "error_codes.h"
#include "Logger.h"
#include "ErrorReporter.h"

#define MOTOR_DEBUG   

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// Helper function to convert MotorErrorCode to a string for logging
const char* motorErrorCodeToString(uint8_t code) {
    switch (code) {
        case MotorErrorCode::NO_FAULT: return "No Fault";
        case MotorErrorCode::MOTOR_OVER_TEMPERATURE: return "Motor Over Temperature";
        case MotorErrorCode::OVER_CURRENT: return "Over Current";
        case MotorErrorCode::OVER_VOLTAGE: return "Over Voltage";
        case MotorErrorCode::UNDER_VOLTAGE: return "Under Voltage";
        case MotorErrorCode::ENCODER_FAULT: return "Encoder Fault";
        case MotorErrorCode::MOSFET_OVER_TEMPERATURE: return "MOSFET Over Temperature";
        case MotorErrorCode::MOTOR_STALL: return "Motor Stall";
        default: return "Unknown Error";
    }
}

_Motor::_Motor(config_defs::joint_id id, ExoData *exo_data)
{
    _id = id;
    _is_left = ((uint8_t)this->_id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    _data = exo_data;
    _error = false;

    // Set _motor_data to point to the data specific to this motor.
    switch (utils::get_joint_type(_id))
    {
    case (uint8_t)config_defs::joint_id::knee:
        if (_is_left)
        {
            _motor_data = &(exo_data->left_side.knee.motor);
        }
        else
        {
            _motor_data = &(exo_data->right_side.knee.motor);
        }
        break;

    case (uint8_t)config_defs::joint_id::ankle:
        if (_is_left)
        {
            _motor_data = &(exo_data->left_side.ankle.motor);
        }
        else
        {
            _motor_data = &(exo_data->right_side.ankle.motor);
        }
        break;
    }

#ifdef MOTOR_DEBUG
    logger::println("_Motor::_Motor : Leaving Constructor");
#endif
};

bool _Motor::get_is_left()
{
    return _is_left;
};

config_defs::joint_id _Motor::get_id()
{
    return _id;
};

_CANMotor::_CANMotor(config_defs::joint_id id, ExoData *exo_data)
    : _Motor(id, exo_data)
{
    // Initialize safety limits with safe default values
    _P_MIN = -36000.0f; // Degrees
    _P_MAX = 36000.0f;  // Degrees
    _V_MIN = -5000.0f;  // RPM
    _V_MAX = 5000.0f;   // RPM
    _A_MIN = 0.0f;      // RPM/s^2
    _A_MAX = 20000.0f;  // RPM/s^2
    _I_MIN = -20.0f;    // Amps
    _I_MAX = 20.0f;     // Amps

    JointData *j_data = exo_data->get_joint_with(static_cast<uint8_t>(id));
    if(j_data != nullptr) {
        j_data->motor.kt = this->get_Kt();
    }

#ifdef MOTOR_DEBUG
    logger::println("_CANMotor::_CANMotor : Leaving Constructor");
#endif
};

void _CANMotor::read_data()
{
    bool searching = true;
    uint32_t start = micros();

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        do
        {
            CAN_message_t msg = can->read();

            if (msg.id == uint32_t(_motor_data->id))
            {
                int16_t p_int = (msg.buf[0] << 8) | msg.buf[1];
                int16_t v_int = (msg.buf[2] << 8) | msg.buf[3];
                int16_t i_int = (msg.buf[4] << 8) | msg.buf[5];
                int8_t temp_int = msg.buf[6];
                uint8_t error_code_raw = msg.buf[7];

                // Set data in ExoData object using scaling from the manual
                _motor_data->p = (float)p_int * 0.1f;
                _motor_data->v = (float)v_int * 10.0f;
                _motor_data->i = (float)i_int * 0.01f;
                _motor_data->temperature = temp_int;
                _motor_data->error = static_cast<uint8_t>(error_code_raw);

                // If an error is detected, log it.
                if (_motor_data->error != MotorErrorCode::NO_FAULT) {
                    logger::print("Motor Error Detected: ", LogLevel::Error);
                    logger::println(motorErrorCodeToString(_motor_data->error));
                }

#ifdef MOTOR_DEBUG
                logger::print("_CANMotor::read_data():Got data-");
                logger::print("ID:" + String(uint32_t(_motor_data->id)) + ",");
                logger::print("P:" + String(_motor_data->p) + ",V:" + String(_motor_data->v) + ",I:" + String(_motor_data->i));
                logger::print("\n");
#endif

                _motor_data->timeout_count = 0;
                return;
            }

            searching = ((micros() - start) < _timeout);

        } while (searching);

        _handle_read_failure();
    }
    return;
};

void _CANMotor::on_off()
{
    if (_data->estop || _error)
    {
        _motor_data->is_on = false; 
#ifdef MOTOR_DEBUG
        logger::print("_CANMotor::on_off() : E-stop pulled or error - ");
        logger::print(uint32_t(_motor_data->id));
        logger::print("\n");
#endif
    }
    
};

void _CANMotor::zero()
{
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_ORIGIN_HERE) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 1;
    msg.buf[0] = 0; // Set a temporary origin

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
    }
};

void _CANMotor::check_response() 
{
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on);

    if (_data->user_paused || !active_trial || _data->estop || _error)
    {
        return;
    }

    _measured_current.push(_motor_data->i); 
    if (_measured_current.size() > _current_queue_size)
    {
        _measured_current.pop();
        auto pop_vals = utils::online_std_dev(_measured_current);

        if (pop_vals.second < _variance_threshold) 
        {
#ifdef MOTOR_DEBUG
            logger::println("Motor current variance is low. Check for stall.");
#endif
        }
    }
};

void _CANMotor::set_position(float pos)
{
    _motor_data->p_des = pos;

    float constrained_pos = constrain(pos, _P_MIN, _P_MAX);

    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 4;

    int32_t pos_scaled = (int32_t)(constrained_pos * 100.0f);

    msg.buf[0] = (pos_scaled >> 24) & 0xFF;
    msg.buf[1] = (pos_scaled >> 16) & 0xFF;
    msg.buf[2] = (pos_scaled >> 8) & 0xFF;
    msg.buf[3] = pos_scaled & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();
    }
}

void _CANMotor::set_speed(float rpm)
{
    _motor_data->v_des = rpm;

    float constrained_rpm = constrain(rpm, _V_MIN, _V_MAX);

    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_RPM) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 4;

    int32_t rpm_scaled = (int32_t)constrained_rpm;

    msg.buf[0] = (rpm_scaled >> 24) & 0xFF;
    msg.buf[1] = (rpm_scaled >> 16) & 0xFF;
    msg.buf[2] = (rpm_scaled >> 8) & 0xFF;
    msg.buf[3] = rpm_scaled & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();
    }
}

void _CANMotor::set_position_speed(float pos, float spd, float acc)
{
    _motor_data->p_des = pos;
    _motor_data->v_des = spd;
    _motor_data->a_des = acc;

    float constrained_pos = constrain(pos, _P_MIN, _P_MAX);
    float constrained_spd = constrain(spd, _V_MIN, _V_MAX);
    float constrained_acc = constrain(acc, _A_MIN, _A_MAX);

    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS_SPD) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 8;

    int32_t pos_scaled = (int32_t)(constrained_pos * 100.0f);
    int16_t spd_scaled = (int16_t)(constrained_spd / 10.0f);
    int16_t acc_scaled = (int16_t)(constrained_acc / 10.0f);

    msg.buf[0] = (pos_scaled >> 24) & 0xFF;
    msg.buf[1] = (pos_scaled >> 16) & 0xFF;
    msg.buf[2] = (pos_scaled >> 8) & 0xFF;
    msg.buf[3] = pos_scaled & 0xFF;
    msg.buf[4] = (spd_scaled >> 8) & 0xFF;
    msg.buf[5] = spd_scaled & 0xFF;
    msg.buf[6] = (acc_scaled >> 8) & 0xFF;
    msg.buf[7] = acc_scaled & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();
    }
}

float _CANMotor::get_Kt()
{
    return _Kt;
}

void _CANMotor::set_error()
{
    _error = true;
}

void _CANMotor::set_Kt(float Kt)
{
    _Kt = Kt;
}

void _CANMotor::_handle_read_failure()
{
    logger::println("CAN Motor - Handle Read Failure", LogLevel::Error);
    _motor_data->timeout_count++;
}

AK10::AK10(config_defs::joint_id id, ExoData *exo_data) : 
    _CANMotor(id, exo_data)
{
    _I_MAX = 20.0f;
    _V_MAX = 22.0f; 
    float kt = 0.114;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;
}

AK60_v1_1::AK60_v1_1(config_defs::joint_id id, ExoData *exo_data) : 
    _CANMotor(id, exo_data)
{
    _I_MAX = 13.5f;
    _V_MAX = 23.04f;
    float kt = 0.078;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;
};

AK80::AK80(config_defs::joint_id id, ExoData *exo_data) : 
    _CANMotor(id, exo_data)
{
    _I_MAX = 24.0f;
    _V_MAX = 25.65f;
    float kt = 0.091 * 9;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;
};

AK70::AK70(config_defs::joint_id id, ExoData *exo_data) : 
    _CANMotor(id, exo_data)
{
    _I_MAX = 23.2f;
    _V_MAX = 15.5f;
    float kt = 0.13 * 10;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;
};

#endif
