#include "Motor.h"
#include "CAN.h"
#include "ErrorManager.h"
#include "error_codes.h"
#include "Logger.h"
#include "ErrorReporter.h"
#include "Utilities.h"

#define MOTOR_DEBUG   

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// Helper function to convert MotorErrorCode to a string for logging
const char* motorErrorCodeToString(uint8_t code) {
    switch (code) {
        case (uint8_t)MotorErrorCode::NO_FAULT: return "No Fault";
        case (uint8_t)MotorErrorCode::MOTOR_OVER_TEMPERATURE: return "Motor Over Temperature";
        case (uint8_t)MotorErrorCode::OVER_CURRENT: return "Over Current";
        case (uint8_t)MotorErrorCode::OVER_VOLTAGE: return "Over Voltage";
        case (uint8_t)MotorErrorCode::UNDER_VOLTAGE: return "Under Voltage";
        case (uint8_t)MotorErrorCode::ENCODER_FAULT: return "Encoder Fault";
        case (uint8_t)MotorErrorCode::MOSFET_OVER_TEMPERATURE: return "MOSFET Over Temperature";
        case (uint8_t)MotorErrorCode::MOTOR_STALL: return "Motor Stall";
        default: return "Unknown Error";
    }
}

_Motor::_Motor(config_defs::joint_id id, ExoData *exo_data)
{
    _id = id;
    _is_left = utils::get_is_left(_id);
    _data = exo_data;
    _error = false;
    is_on = false;

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
    logger::println("_Motor::_Motor : Constructor completed");
#endif
}

bool _Motor::get_is_left()
{
    return _is_left;
}

config_defs::joint_id _Motor::get_id()
{
    return _id;
}

//****************************************************
// CAN Motor Implementation
//****************************************************

_CANMotor::_CANMotor(config_defs::joint_id id, ExoData *exo_data)
    : _Motor(id, exo_data)
{
    // Initialize safety limits with safe default values
    _P_MIN = -36000.0f; // Degrees
    _P_MAX = 36000.0f;  // Degrees
    _V_MIN = -5000.0f;  // RPM
    _V_MAX = 5000.0f;   // RPM
    _A_MIN = 0.0f;      // RPM/s^2
    _A_MAX = 2000.0f;  // RPM/s^2
    _I_MIN = -20.0f;    // Amps
    _I_MAX = 20.0f;     // Amps

    JointData *j_data = exo_data->get_joint_with(static_cast<uint8_t>(id));
    if(j_data != nullptr) {
        j_data->motor.kt = this->get_Kt();
    }

#ifdef MOTOR_DEBUG
    logger::println("_CANMotor::_CANMotor : Constructor completed");
#endif
}

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
                if (_motor_data->error != (uint8_t)MotorErrorCode::NO_FAULT) {
                    logger::print("Motor Error Detected: ");
                    logger::println(motorErrorCodeToString(_motor_data->error));
                }

#ifdef MOTOR_DEBUG
                logger::print("_CANMotor::read_data(): Got data - ");
                logger::print("ID: ");
                logger::print(uint32_t(_motor_data->id));
                logger::print(", P: ");
                logger::print(_motor_data->p);
                logger::print(", V: ");
                logger::print(_motor_data->v);
                logger::print(", I: ");
                logger::println(_motor_data->i);
#endif

                _motor_data->timeout_count = 0;
                return;
            }

            searching = ((micros() - start) < _timeout);

        } while (searching);

        _handle_read_failure();
    }
    return;
}

void _CANMotor::set_position(float pos)
{
    _motor_data->p_des = pos;
    float constrained_pos = constrain(pos, _P_MIN, _P_MAX);
    
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 4;

    // Convert position to int32_t (0.01 degree resolution)
    int32_t pos_cmd = (int32_t)(constrained_pos * 10000.0f);
    msg.buf[0] = (pos_cmd >> 24) & 0xFF;
    msg.buf[1] = (pos_cmd >> 16) & 0xFF;
    msg.buf[2] = (pos_cmd >> 8) & 0xFF;
    msg.buf[3] = pos_cmd & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();

#ifdef MOTOR_DEBUG
        logger::print("Motor Position Command - Joint: ");
        logger::print(_is_left ? "Left " : "Right ");
        logger::print(utils::get_joint_type(_id) == (uint8_t)config_defs::joint_id::knee ? "Knee" : "Ankle");
        logger::print(", Pos: ");
        logger::print(constrained_pos);
        logger::println("°");
#endif
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

    // Convert RPM to int32_t (0.1 RPM resolution)
    int32_t rpm_cmd = (int32_t)(constrained_rpm * 10.0f);
    msg.buf[0] = (rpm_cmd >> 24) & 0xFF;
    msg.buf[1] = (rpm_cmd >> 16) & 0xFF;
    msg.buf[2] = (rpm_cmd >> 8) & 0xFF;
    msg.buf[3] = rpm_cmd & 0xFF;

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

    // Convert position to int32_t (0.01 degree resolution)
    int32_t pos_cmd = (int32_t)(constrained_pos * 10000.0f);
    msg.buf[0] = (pos_cmd >> 24) & 0xFF;
    msg.buf[1] = (pos_cmd >> 16) & 0xFF;
    msg.buf[2] = (pos_cmd >> 8) & 0xFF;
    msg.buf[3] = pos_cmd & 0xFF;

    // Convert speed to int16_t (0.1 RPM resolution)
    int16_t spd_cmd = (int16_t)(constrained_spd / 10.0f);
    msg.buf[4] = (spd_cmd >> 8) & 0xFF;
    msg.buf[5] = spd_cmd & 0xFF;

    // Convert acceleration to int16_t (0.1 RPM/s resolution)
    int16_t acc_cmd = (int16_t)(constrained_acc / 10.0f);
    msg.buf[6] = (acc_cmd >> 8) & 0xFF;
    msg.buf[7] = acc_cmd & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();
    }
}

void _CANMotor::set_current(float current)
{
    // Store the desired current
    _motor_data->i_des = current;
    
    // Apply safety limits 
    float constrained_current = constrain(current, _I_MIN, _I_MAX);
    
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_CURRENT) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 4;

    // Convert current to int32_t (0.001 A resolution)
    int32_t current_cmd = (int32_t)(constrained_current * 1000.0f);
    msg.buf[0] = (current_cmd >> 24) & 0xFF;
    msg.buf[1] = (current_cmd >> 16) & 0xFF;
    msg.buf[2] = (current_cmd >> 8) & 0xFF;
    msg.buf[3] = current_cmd & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();
    }

#ifdef MOTOR_DEBUG
    logger::print("_CANMotor::set_current() : Current Command = ");
    logger::print(constrained_current);
    logger::print("A, Motor ID = ");
    logger::println(uint32_t(_motor_data->id));
#endif
}

void _CANMotor::set_current_brake(float current, float brake)
{

    // Store the desired current
    _motor_data->i_des = current;
    
    // Apply safety limits
    float constrained_current = constrain(current, _I_MIN, _I_MAX);
    float constrained_brake_current = constrain(brake, 0.0f, 13.5f); // 0-13.5A brake current

    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_CURRENT_BRAKE) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 4; // 문서에 따르면 brake current만 전송 (4바이트)

    // Convert brake current to int32_t (0.001 A resolution) - 문서 방식과 동일
    int32_t brake_current_cmd = (int32_t)(constrained_brake_current * 1000.0f);
    msg.buf[0] = (brake_current_cmd >> 24) & 0xFF;
    msg.buf[1] = (brake_current_cmd >> 16) & 0xFF;
    msg.buf[2] = (brake_current_cmd >> 8) & 0xFF;
    msg.buf[3] = brake_current_cmd & 0xFF;

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();
        check_response();
    }

#ifdef MOTOR_DEBUG
    logger::print("_CANMotor::set_current_brake() : Brake Current = ");
    logger::print(constrained_brake_current);
    logger::print("A, Motor ID = ");
    logger::println(uint32_t(_motor_data->id));
#endif
}

void _CANMotor::on_off()
{
    // Check for emergency stop or error conditions
    if (_data->estop || _error)
    {
        // Emergency/Error 상황: Current Brake로 안전한 정지
        // Apply brake BEFORE setting is_on = false so CAN message sends
        if (_motor_data->is_on) {
            float safety_brake_current = 1.0f; // 1A 매우 부드러운 브레이크
            set_current_brake(0.0f, safety_brake_current);
            
#ifdef MOTOR_DEBUG
            logger::print("_CANMotor::on_off() : SAFETY BRAKE - ");
            logger::print(safety_brake_current);
            logger::print("A (E-stop: ");
            logger::print(_data->estop ? "YES" : "NO");
            logger::print(", Error: ");
            logger::print(_error ? "YES" : "NO");
            logger::print("), Motor ID: ");
            logger::println(uint32_t(_motor_data->id));
#endif
        }
        
        _motor_data->is_on = false; 
        is_on = false;
    }
    else
    {
        uint16_t exo_status = _data->get_status();
        
        bool trial_active = (exo_status == status_defs::messages::trial_on);
        
        if (trial_active && !_data->user_paused)
        {
            _motor_data->is_on = true;
            is_on = true;
        }
        else
        {
            _motor_data->is_on = false;
            is_on = false;
        }
    }
}

void _CANMotor::zero()
{
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_ORIGIN_HERE) << 8) | uint32_t(_motor_data->id);
    msg.flags.extended = 1;
    msg.len = 1;
    msg.buf[0] = 0; // Set origin here

    if (_motor_data->is_on)
    {
        CAN *can = CAN::getInstance();
        can->send(msg);
        read_data();

#ifdef MOTOR_DEBUG
        logger::print("Motor zeroed - ID: ");
        logger::println(uint32_t(_motor_data->id));
#endif
    }
}

void _CANMotor::check_response() 
{
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on);

    if (_data->user_paused || !active_trial || _data->estop || _error)
    {
        return;
    }

    // Monitor current for stall detection - 큐 크기 관리 개선
    if (_measured_current.size() >= _current_queue_size)
    {
        _measured_current.pop();  // 먼저 오래된 데이터 제거
    }
    _measured_current.push(_motor_data->i);  // 새 데이터 추가
    
    // 충분한 데이터가 있을 때만 분석
    if (_measured_current.size() == _current_queue_size)
    {
        auto stats = utils::online_std_dev(_measured_current);

        if (stats.second < _variance_threshold) 
        {
#ifdef MOTOR_DEBUG
            logger::print("Motor potentially stalled - Low current variance: ");
            logger::println(stats.second);
#endif
            // Motor might be stalled - could trigger error handling here
        }
    }
}

void _CANMotor::set_error()
{
    _error = true;
    _motor_data->is_on = false;
    is_on = false;

#ifdef MOTOR_DEBUG
    logger::print("Motor error set - ID: ");
    logger::println(uint32_t(_motor_data->id));
#endif
}

float _CANMotor::get_Kt()
{
    return _Kt;
}

void _CANMotor::set_Kt(float Kt)
{
    _Kt = Kt;
}

void _CANMotor::_handle_read_failure()
{
    _motor_data->timeout_count++;
    
#ifdef MOTOR_DEBUG
    logger::print("Motor read timeout - ID: ");
    logger::print(uint32_t(_motor_data->id));
    logger::print(", Count: ");
    logger::println(_motor_data->timeout_count);
#endif

    // If too many timeouts, disable motor
    if (_motor_data->timeout_count > _motor_data->timeout_count_max)
    {
        set_error();
    }
}

//****************************************************
// Specific Motor Implementations
//****************************************************

AK10::AK10(config_defs::joint_id id, ExoData *exo_data) : _CANMotor(id, exo_data)
{
    set_Kt(0.146f); // AK10 torque constant
    
    // AK10 specific limits
    _P_MIN = -10.5f * 360.0f; // -12.5 revolutions
    _P_MAX = 10.5f * 360.0f;  // 12.5 revolutions
    _V_MIN = -500.0f;          // -50 RPM
    _V_MAX = 500.0f;           // 50 RPM
    _I_MIN = -24.0f;          // -24A
    _I_MAX = 24.0f;           // 24A

#ifdef MOTOR_DEBUG
    logger::println("AK10 motor initialized");
#endif
}

AK60_v1_1::AK60_v1_1(config_defs::joint_id id, ExoData *exo_data) : _CANMotor(id, exo_data)
{
    set_Kt(0.068f); // AK60 torque constant
    
    // AK60 specific limits
    _P_MIN = -10.0f * 360.0f;
    _P_MAX = 10.0f * 360.0f;
    _V_MIN = -4500.0f;
    _V_MAX = 4500.0f;
    _I_MIN = -23.0f;
    _I_MAX = 23.0f;

#ifdef MOTOR_DEBUG
    logger::println("AK60 v1.1 motor initialized");
#endif
}

AK80::AK80(config_defs::joint_id id, ExoData *exo_data) : _CANMotor(id, exo_data)
{
    set_Kt(0.1f); // AK80 torque constant
    
    // AK80 specific limits
    _P_MIN = -12.5f * 360.0f;
    _P_MAX = 12.5f * 360.0f;
    _V_MIN = -25.6f;
    _V_MAX = 25.6f;
    _I_MIN = -23.0f;
    _I_MAX = 23.0f;

#ifdef MOTOR_DEBUG
    logger::println("AK80 motor initialized");
#endif
}

AK70::AK70(config_defs::joint_id id, ExoData *exo_data) : _CANMotor(id, exo_data)
{
    set_Kt(0.093f); // AK70 torque constant
    
    // AK70 specific limits
    _P_MIN = -12.5f * 360.0f;
    _P_MAX = 12.5f * 360.0f;
    _V_MIN = -25.6f;
    _V_MAX = 25.6f;
    _I_MIN = -23.0f;
    _I_MAX = 23.0f;

#ifdef MOTOR_DEBUG
    logger::println("AK70 motor initialized");
#endif
}

#endif