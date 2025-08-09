/*
 * P. Stegall Jan. 2022
 */

#include "Arduino.h"

#include "Motor.h"
#include "CAN.h"
#include "ErrorManager.h"
#include "error_codes.h"
#include "Logger.h"
#include "ErrorReporter.h"
#include "error_codes.h"
// #define MOTOR_DEBUG           //Uncomment if you want to print debug statments to the serial monitor

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

_Motor::_Motor(config_defs::joint_id id, ExoData *exo_data, int enable_pin)
{
    _id = id;
    _is_left = ((uint8_t)this->_id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    _data = exo_data;
    _enable_pin = enable_pin;
    _prev_motor_enabled = false;
    _prev_on_state = false;

#ifdef MOTOR_DEBUG
    logger::print("_Motor::_Motor : _enable_pin = ");
    logger::print(_enable_pin);
    logger::print("\n");
#endif

    pinMode(_enable_pin, OUTPUT);

    // Set _motor_data to point to the data specific to this motor.
    switch (utils::get_joint_type(_id)) // utils::get_joint_type() 함수는 joint_id를 받아서 해당 관절의 타입을 반환하는 함수
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

//  config_defs::joint_id id 으로 정의되어 있고, _id = id 으로 정의되어 있음.
config_defs::joint_id _Motor::get_id() // 반환타입 (config_defs::joint_id) 는 enum class로 정의되어 있음 그리고 _Motor 클래스의 get_id라는 함수는 _id 변수를 반환함
{
    return _id;
};

/*
 * Constructor for the CAN Motor.
 * We are using multilevel inheritance, so we have a general motor type, which is inherited by the CAN (e.g. TMotor) or other type (e.g. Maxon) since models within these types will share communication protocols, which is then inherited by the specific motor model (e.g. AK60), which may have specific torque constants etc.
 */
_CANMotor::_CANMotor(config_defs::joint_id id, ExoData *exo_data, int enable_pin) // Constructor: type is the motor type
    : _Motor(id, exo_data, enable_pin)
{
    _KP_MIN = 0.0f;
    _KP_MAX = 500.0f;
    _KD_MIN = 0.0f;
    _KD_MAX = 5.0f;
    _P_MAX = 12.5f;

    JointData *j_data = exo_data->get_joint_with(static_cast<uint8_t>(id));
    j_data->motor.kt = this->get_Kt(); // 관절 데이터의 motor.kt 변수에 접근해서 그 kt를 데려옴

    _enable_response = false;

#ifdef MOTOR_DEBUG
    logger::println("_CANMotor::_CANMotor : Leaving Constructor");
#endif
};

void _CANMotor::transaction(float torque) // transaction(torque): 모터에 토크 명령을 보내고, 모터의 현재 상태를 읽는 함수
{
    // Send data and read response
    send_data(torque); // send_data(torque): 모터에 토크 명령을 보냅니다 : 이 시스템에서는 Torque Control 사용
    read_data();       // 모터로부터 현재 상태 데이터를 읽습니다.
    check_response();  // check_response(): 모터가 활성화 명령에 응답했는지 확인합니다. 응답하지 않으면 에러를 발생시킴
};

void _CANMotor::read_data() // 모터드라이버가 보낸 데이터를 읽는 함수
{
    // Read data from motor
    bool searching = true;
    uint32_t start = micros();

    // Only send and receive data if enabled
    if (_motor_data->enabled)
    {
        CAN *can = can->getInstance();
        do
        {

            CAN_message_t msg = can->read();
            if (msg.buf[0] == uint32_t(_motor_data->id))
            {
                // Unpack data
                uint32_t p_int = (msg.buf[1] << 8) | msg.buf[2]; // 8칸 옆으로 옮겨야 16bit 정수로 변환됨
                uint32_t v_int = (msg.buf[3]) << 4 | (msg.buf[4] >> 4);
                uint32_t i_int = ((msg.buf[4] & 0xF) << 8) | msg.buf[5]; // 0xF 가 00001111 인데,  & 연산을 통해 하위의 4비트만 가져오고 8칸 옆으로 옮기고 거기에 buf[5]값을 더함

                // Set data in ExoData object
                _motor_data->p = _uint_to_float(p_int, -_P_MAX, _P_MAX, 16);
                _motor_data->v = _uint_to_float(v_int, -_V_MAX, _V_MAX, 12);
                _motor_data->i = _uint_to_float(i_int, -_I_MAX, _I_MAX, 12);

#ifdef MOTOR_DEBUG
                logger::print("_CANMotor::read_data():Got data-");
                logger::print("ID:" + String(uint32_t(_motor_data->id)) + ",");
                logger::print("P:" + String(_motor_data->p) + ",V:" + String(_motor_data->v) + ",I:" + String(_motor_data->i));
                logger::print("\n");
#endif

                // Reset timout_count because we got a valid message
                _motor_data->timeout_count = 0;
                return;
            }

            searching = ((micros() - start) < _timeout); // 정해진 시간(_timeout) 동안 CAN 버스로부터 메시지가 들어오는지 계속 확인

        } while (searching); // searching이 계속 ture인 동안 계속 반복하는 것, do-while문이기 때문에 searching이 false가 되면 종료됨 종료되면 timeout이 발생한 것이기 때문에 아래 함수 실행됨

        // If we get here, we timed out
        _handle_read_failure();
    }
    return;
};

void _CANMotor::send_data(float torque) // 모터 드라이버에 명령을 내리는 함수 , 토크 컨트롤임
{
#ifdef MOTOR_DEBUG
    logger::print("Sending data: ");
    logger::print(uint32_t(_motor_data->id));
    logger::print("\n");
#endif

    _motor_data->t_ff = torque;
    const float current = torque / get_Kt();

    // Read data from ExoData object, constraint it, and package it
    float p_sat = constrain(_motor_data->p_des, -_P_MAX, _P_MAX);
    float v_sat = constrain(_motor_data->v_des, -_V_MAX, _V_MAX);
    float kp_sat = constrain(_motor_data->kp, _KP_MIN, _KP_MAX);
    float kd_sat = constrain(_motor_data->kd, _KD_MIN, _KD_MAX);
    float i_sat = constrain(current, -_I_MAX, _I_MAX);
    _motor_data->last_command = i_sat;
    uint32_t p_int = _float_to_uint(p_sat, -_P_MAX, _P_MAX, 16);
    uint32_t v_int = _float_to_uint(v_sat, -_V_MAX, _V_MAX, 12);
    uint32_t kp_int = _float_to_uint(kp_sat, _KP_MIN, _KP_MAX, 12);
    uint32_t kd_int = _float_to_uint(kd_sat, _KD_MIN, _KD_MAX, 12);
    uint32_t i_int = _float_to_uint(i_sat, -_I_MAX, _I_MAX, 12);
    CAN_message_t msg;
    msg.id = uint32_t(_motor_data->id);
    msg.buf[0] = p_int >> 8; // 8비트를 왼쪽으로 이동시켜서 상위 8비트를 저장
    msg.buf[1] = p_int & 0xFF;
    msg.buf[2] = v_int >> 4;
    msg.buf[3] = ((v_int & 0xF) << 4) | (kp_int >> 8);
    msg.buf[4] = kp_int & 0xFF;
    msg.buf[5] = kd_int >> 4;
    msg.buf[6] = ((kd_int & 0xF) << 4) | (i_int >> 8);
    msg.buf[7] = i_int & 0xFF;

    // logger::print("_CANMotor::send_data::t_sat:: ");
    // logger::print(t_sat);
    // logger::print("\n");
    // logger::print("T:"+String(t_sat)+",");

    // Only send messages if enabled
    if (_motor_data->enabled)
    {
        // Set data in motor
        CAN *can = can->getInstance();
        can->send(msg);
    }
    return;
};

void _CANMotor::check_response() // 모터가 명령에 따라 실제로 움직이고 있는지를 전류의 변화량을 통해 간접적으로 확인하는 안전장치
{
    // Only run if the motor is supposed to be enabled
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on) ||
                        (exo_status == status_defs::messages::fsr_calibration) ||
                        (exo_status == status_defs::messages::fsr_refinement);

    if (_data->user_paused || !active_trial || _data->estop || _error)
    {
        return;
    }

    // Measured current variance should be non-zero
    _measured_current.push(_motor_data->i); // 최근의 전류값들을 모아가지고 데이터를 분석하는 곳, measured_current 큐에 현재 모터의 전류 값을 추가합니다.

    // 모터가 멈췄거나 응답이 없는 것으로 의심될 때 강제로 재활성화하는 안전 로직
    if (_measured_current.size() > _current_queue_size)
    {
        _measured_current.pop();
        auto pop_vals = utils::online_std_dev(_measured_current); // pop_vals.second에는 표준편차 값이 저장

        if (pop_vals.second < _variance_threshold) // 계산된 표준편차가 미리 설정된 매우 작은 값보다 작은 것은 전류값에 거의 변화가 없다는 것
        {
            _motor_data->enabled = true; // 모터를 다시 활성화합니다.
            enable(true);                // 강제로 enable 함수를 호출하여 모터를 활성화합니다.
        }
    }
};

void _CANMotor::on_off() // on_off() 함수는 모터 드라이버의 전원을 물리적으로 켜거나 끄는 역할
{
    if (_data->estop || _error)
    {
        _motor_data->is_on = false; // 만약 비상정지(estop)가 눌렸거나 다른 에러(_error)가 발생한 상태라면, 모터의 전원 상태(_motor_data->is_on)를 강제로 false(꺼짐)로 바꿉

        // logger::print("_CANMotor::on_off(bool is_on) : E-stop pulled - ");
        // logger::print(uint32_t(_motor_data->id));
        // logger::print("\n");
    }

    // 현재 원하는 전원 상태(is_on)가 이전 상태(_prev_on_state)와 다를 때, 즉 상태가 방금 바뀌었을 때만
    if (_prev_on_state != _motor_data->is_on) // If was here to save time, can be removed if making problems, or add overide
    {
        if (_motor_data->is_on)
        {
            digitalWrite(_enable_pin, logic_micro_pins::motor_enable_on_state); // is_on이 true이면, digitalWrite 함수를 이용해 모터 활성화 핀(_enable_pin)에 HIGH 신호를 보냅니다.

            // logger::print("_CANMotor::on_off(bool is_on) : Power on- ");
            // logger::print(uint32_t(_motor_data->id));
            // logger::print("\n");
        }
        else
        {
            digitalWrite(_enable_pin, logic_micro_pins::motor_enable_off_state);

            // logger::print("_CANMotor::on_off(bool is_on) : Power off- ");
            // logger::print(uint32_t(_motor_data->id));
            // logger::print("\n");
        }
    }
    _prev_on_state = _motor_data->is_on;

#ifdef HEADLESS  // HEADLESS 모드는 GUI 없이 독립적으로 작동하는 모드임. 2초의 지연시간은 모터드라이버가 켜지고 안정화되는 시간을 주기 위해 설정
    delay(2000); // Two second delay between motor's turning on and enabeling, we've run into some issues with enabling while in headless mode if this delay is not present.
#endif
};

bool _CANMotor::enable() // 단순히 enable(false)를 호출하는 단축키 역할
{
    return enable(false);
};

bool _CANMotor::enable(bool overide)
{
#ifdef MOTOR_DEBUG
    //  logger::print(_prev_motor_enabled);
    //  logger::print("\t");
    //  logger::print(_motor_data->enabled);
    //  logger::print("\t");
    //  logger::print(_motor_data->is_on);
    //  logger::print("\n");
#endif

    // Only change the state and send messages if the enabled state has changed.
    if ((_prev_motor_enabled != _motor_data->enabled) || overide || !_enable_response)
    {
        CAN_message_t msg; // '제어 모드 진입/해제'**를 의미하는 특수 명령어
        /* Special CAN Codes
        Enter Motor Control Mode: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0XFC }
        Exit Motor Control Mode: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFD }
        Set Current Motor Position as zero position: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFE } */
        // 0xFC, 0xFD, 0xFE는 각각 모터 제어 모드 진입, 해제, 현재 위치를 제로로 설정하는 명령어임
        msg.id = uint32_t(_motor_data->id);
        msg.buf[0] = 0xFF;
        msg.buf[1] = 0xFF;
        msg.buf[2] = 0xFF;
        msg.buf[3] = 0xFF;
        msg.buf[4] = 0xFF;
        msg.buf[5] = 0xFF;
        msg.buf[6] = 0xFF;

        if (_motor_data->enabled && !_error && !_data->estop)
        {
            msg.buf[7] = 0xFC;
        }
        else
        {
            _enable_response = false;
            msg.buf[7] = 0xFD;
        }

        CAN *can = can->getInstance();
        can->send(msg);
        delayMicroseconds(500);
        read_data();

        if (_motor_data->timeout_count == 0)
        {
            _enable_response = true;
        }
    }

    _prev_motor_enabled = _motor_data->enabled;
    return _enable_response;
};

void _CANMotor::zero()
{
    CAN_message_t msg;
    msg.id = uint32_t(_motor_data->id);
    msg.buf[0] = 0xFF;
    msg.buf[1] = 0xFF;
    msg.buf[2] = 0xFF;
    msg.buf[3] = 0xFF;
    msg.buf[4] = 0xFF;
    msg.buf[5] = 0xFF;
    msg.buf[6] = 0xFF;
    msg.buf[7] = 0xFE;
    CAN *can = can->getInstance();
    can->send(msg);

    read_data();
};

float _CANMotor::get_Kt()
{
    return _Kt;
};

void _CANMotor::set_error()
{
    _error = true;
};

void _CANMotor::set_Kt(float Kt)
{
    _Kt = Kt;
};

void _CANMotor::_handle_read_failure()
{
    logger::println("CAN Motor - Handle Read Failure", LogLevel::Error);
    _motor_data->timeout_count++;
};

float _CANMotor::_float_to_uint(float x, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    unsigned int pgg = 0;
    if (bits == 12)
    {
        pgg = (unsigned int)((x - offset) * 4095.0 / span);
    }
    if (bits == 16)
    {
        pgg = (unsigned int)((x - offset) * 65535.0 / span);
    }
    return pgg;
};

float _CANMotor::_uint_to_float(unsigned int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    float pgg = 0;
    if (bits == 12)
    {
        pgg = ((float)x_int) * span / 4095.0 + offset;
    }
    if (bits == 16)
    {
        pgg = ((float)x_int) * span / 65535.0 + offset;
    }
    return pgg;
};

//**************************************
/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK60::AK60(config_defs::joint_id id, ExoData *exo_data, int enable_pin) : // Constructor: type is the motor type
                                                                          _CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 22.0f;
    _V_MAX = 41.87f;

    float kt = 0.068 * 6;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt; // motor.kt는 MotorData 구조체의 멤버 변수로, 모터의 토크 상수(Kt)를 저장하는 역할을 합니다.

#ifdef MOTOR_DEBUG
    logger::println("AK60::AK60 : Leaving Constructor");
#endif
};

/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK60_v1_1::AK60_v1_1(config_defs::joint_id id, ExoData *exo_data, int enable_pin) : // Constructor: type is the motor type
                                                                                    _CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 13.5f;  // Cubemars에서 여기를 set_current_limit 으로 설정을 했음 (왜 13.5f인지 모르겠음)
    _V_MAX = 23.04f; // 아마 배터리의 22.2V 공칭 전압보다는 높지만 완충보다는 낮은 상태?

    float kt = 0.1725 * 6; // We set KT to 0.1725 * 6 whcih differs from the manufacturer's stated KT, that's because they are wrong (This has been validated mulitple ways). We only have validated for this version as we use open loop at the hip with these, other motors are used with closed loop and thus are corrected in real-time. We recommend validating these KTs if using for open loop.
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

#ifdef MOTOR_DEBUG
    logger::println("AK60_v1_1::AK60_v1_1 : Leaving Constructor");
#endif
};

/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK80::AK80(config_defs::joint_id id, ExoData *exo_data, int enable_pin) : // Constructor: type is the motor type
                                                                          _CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 24.0f;
    _V_MAX = 25.65f;

    float kt = 0.091 * 9;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

#ifdef MOTOR_DEBUG
    logger::println("AK80::AK80 : Leaving Constructor");
#endif
};

/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK70::AK70(config_defs::joint_id id, ExoData *exo_data, int enable_pin) : // Constructor: type is the motor type
                                                                          _CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 23.2f;
    _V_MAX = 15.5f;

    float kt = 0.13 * 10;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

#ifdef MOTOR_DEBUG
    logger::println("AK70::AK70 : Leaving Constructor");
#endif
};

/*
 * Constructor for the PWM (Maxon) Motor.
 * We are using multilevel inheritance, so we have a general motor type, which is inherited by the PWM (e.g. Maxon) or other type (e.g. Maxon) since models within these types will share communication protocols, which is then inherited by the specific motor model, which may have specific torque constants etc.
 */
MaxonMotor::MaxonMotor(config_defs::joint_id id, ExoData *exo_data, int enable_pin) // Constructor: type is the motor type
    : _Motor(id, exo_data, enable_pin)
{
    JointData *j_data = exo_data->get_joint_with(static_cast<uint8_t>(id));

#ifdef MOTOR_DEBUG
    logger::println("MaxonMotor::MaxonMotor: Leaving Constructor");
#endif
};

void MaxonMotor::transaction(float torque)
{
    // Send data
    send_data(torque);

    // Only enable the motor when it is an active trial
    master_switch();

    if (_motor_data->enabled)
    {
        maxon_manager(true); // Monitors for and corrects motor resetting error if the system is operational.
    }
    else
    {
        maxon_manager(false); // Reset the motor error detection function, in case user pauses device in middle of error event
    }

    // Serial.print("\nRight leg MaxonMotor::transaction(float torque)  |  torque = ");
    // Serial.print(torque);
};

bool MaxonMotor::enable()
{
    return true; // This function is currently bypassed for this motor at the moment.
};

bool MaxonMotor::enable(bool overide)
{
    // Only change the state and send messages if the enabled state (used as a master switch for this motor) has changed.
    if ((_prev_motor_enabled != _motor_data->enabled) || overide)
    {
        if (_motor_data->enabled) //_motor_data->enabled is controlled by the GUI
        {
            // Enable motor
            digitalWrite(_enable_pin, HIGH); // Relocate in the future
        }

        _enable_response = true;
    }

    if (!overide) // When enable(false), send the disable motor command, set the analogWrite resolution, and send 50% PWM command
    {
        _enable_response = false;

        // Disable motor, the message after this shouldn't matter as the power is cut, and the send() doesn't send a message if not enabled.
        digitalWrite(_enable_pin, LOW);
        analogWrite(_ctrl_right_pin, _pwm_neutral_val);
        analogWrite(_ctrl_left_pin, _pwm_neutral_val);
    }

    if (!_motor_data->enabled) //_motor_data->enabled is controlled by the GUI
    {
        // Disable motor
        digitalWrite(_enable_pin, LOW); // Relocate in the future
    }

    _prev_motor_enabled = _motor_data->enabled;

    return _enable_response;

#ifdef MOTOR_DEBUG
    logger::print(_prev_motor_enabled);
    logger::print("\t");
    logger::print(_motor_data->enabled);
    logger::print("\t");
    logger::print(_motor_data->is_on);
    logger::print("\n");
#endif
};

void MaxonMotor::send_data(float torque) // Always send motor command regardless of the motor "enable" status
{
#ifdef MOTOR_DEBUG
    logger::print("Sending data: ");
    logger::print(uint32_t(_motor_data->id));
    logger::print("\n");
#endif

    _motor_data->t_ff = torque;
    _motor_data->last_command = torque;

    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on) ||
                        (exo_status == status_defs::messages::fsr_calibration) ||
                        (exo_status == status_defs::messages::fsr_refinement);

    if (_data->user_paused || !active_trial || _data->estop) // Ignores the exo error handler for the moment
    {
        analogWrite(_ctrl_left_pin, _pwm_neutral_val);  // Set 50% PWM (0 current)
        analogWrite(_ctrl_right_pin, _pwm_neutral_val); // Set 50% PWM (0 current)
    }
    else
    {
        // Constrain the motor pwm command
        uint16_t post_fuse_torque = max(_pwm_l_bound, _pwm_neutral_val + (torque));               // Set the lowest allowed PWM command
        post_fuse_torque = min(_pwm_u_bound, post_fuse_torque);                                   // Set the highest allowed PWM command
        analogWrite((_motor_data->is_left ? _ctrl_left_pin : _ctrl_right_pin), post_fuse_torque); // Send the motor command to the motor driver
    }
};

void MaxonMotor::master_switch()
{
    // Only run if the motor is supposed to be enabled
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on) ||
                        (exo_status == status_defs::messages::fsr_calibration) ||
                        (exo_status == status_defs::messages::fsr_refinement);

    if (_data->user_paused || !active_trial || _data->estop)
    {
        pinMode(_err_left_pin, INPUT_PULLUP);
        pinMode(_err_right_pin, INPUT_PULLUP);
        pinMode(_current_left_pin, INPUT);
        pinMode(_current_right_pin, INPUT);
        analogWriteResolution(12);
        analogWriteFrequency(_ctrl_left_pin, 5000);
        analogWriteFrequency(_ctrl_right_pin, 5000);

        //_motor_data->enabled = false;
        enable(false);
    }
    else
    {
        //_motor_data->enabled = true;
        enable(true);
    }
};

// Our implementation of the Maxon motor including the ec motor and the Escon 50_8 Motor Controller would occasionally cause 50_8 to enter error mode, with "Over current" being one of the errors.
// To address this issue, we have developed a solution contained in maxon_manager() below.
void MaxonMotor::maxon_manager(bool manager_active)
{
    // Initialize variables when switch is set to false, run the error detection and rest code when switch is set to true.
    if (!manager_active)
    {
        // Reset Maxon motor reset utilities
        do_scan4maxon_err_left = true;
        maxon_counter_active_left = false;
        do_scan4maxon_err_right = true;
        maxon_counter_active_right = false;
    }
    else
    {
        unsigned long maxon_reset_current_t = millis();

        // Scan for left motor error
        if ((do_scan4maxon_err_left) && (!digitalRead(_err_left_pin)))
        {
            do_scan4maxon_err_left = false;
            maxon_counter_active_left = true;
            zen_millis_left = maxon_reset_current_t;
        }

        // Left motor reset
        if (maxon_counter_active_left)
        {
            // Two iterations after maxon_counter_actie = true, de-enable motor
            if (maxon_reset_current_t - zen_millis_left >= 2)
            {
                enable(false);
            }

            // Ten iterations after maxon_counter_actie = true, re-enable motor
            if (maxon_reset_current_t - zen_millis_left >= 10)
            {
                enable(true);
            }

            // Thirty iterations after maxon_counter_actie = true, start scanning for error again
            if (maxon_reset_current_t - zen_millis_left >= 30)
            {
                do_scan4maxon_err_left = true;
                maxon_counter_active_left = false;
                _motor_data->maxon_plotting_scalar = -1 * _motor_data->maxon_plotting_scalar;
            }
        }

        // Scan for right motor error
        if ((do_scan4maxon_err_right) && (!digitalRead(_err_right_pin)))
        {
            do_scan4maxon_err_right = false;
            maxon_counter_active_right = true;
            zen_millis_right = maxon_reset_current_t;
        }

        // Right motor reset
        if (maxon_counter_active_right)
        {
            // Two iterations after maxon_counter_actie = true, de-enable motor
            if (maxon_reset_current_t - zen_millis_right >= 2)
            {
                enable(false);
            }

            // Ten iterations after maxon_counter_actie = true, re-enable motor
            if (maxon_reset_current_t - zen_millis_right >= 10)
            {
                enable(true);
            }

            // Thirty iterations after maxon_counter_actie = true, start scanning for error again
            if (maxon_reset_current_t - zen_millis_right >= 30)
            {
                do_scan4maxon_err_right = true;
                maxon_counter_active_right = false;
                _motor_data->maxon_plotting_scalar = -1 * _motor_data->maxon_plotting_scalar;
            }
        }
    }
};

#endif