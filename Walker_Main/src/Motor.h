#ifndef Motor_h
#define Motor_h

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "ExoData.h"
#include "ParseIni.h"
#include "Board.h"
#include "Utilities.h"

#include <stdint.h>

class _Motor
{
public:
    _Motor(config_defs::joint_id id, ExoData *exo_data, int enable_pin);
    virtual ~_Motor() {};

    virtual void read_data() = 0;

    virtual void send_data(float torque) = 0;

    virtual void transaction(float torque) = 0;

    virtual void on_off() = 0;

    virtual bool enable() = 0;

    virtual bool enable(bool overide) = 0;

    virtual void zero() = 0;

    virtual bool get_is_left(); //

    virtual config_defs::joint_id get_id();

    virtual float get_Kt() = 0;

    virtual void set_error() = 0; /**< Sets the error flag for the motor. */

protected:
    config_defs::joint_id _id; /**< Motor ID */
    bool _is_left;
    ExoData *_data;
    MotorData *_motor_data;
    int _enable_pin;
    bool _prev_motor_enabled;
    bool _prev_on_state;
    bool _error = false;
    float _Kt;
};
class NullMotor : public _Motor
{
public:
    NullMotor(config_defs::joint_id id, ExoData *exo_data, int enable_pin) : _Motor(id, exo_data, enable_pin) {};
    void read_data() {};
    void send_data(float torque) {};
    void transaction(float torque) {};
    void on_off() {};
    bool enable() { return true; };
    bool enable(bool overide) { return true; };
    void zero() {};
    float get_Kt() { return 0.0; };
    void set_error() {};
};

class MaxonMotor : public _Motor
{
public:
    MaxonMotor(config_defs::joint_id id, ExoData *exo_data, int enable_pin);
    void transaction(float torque);
    void read_data() {};
    void send_data(float torque);
    void on_off() {};
    bool enable();
    bool enable(bool overide);
    void zero() {};
    float get_Kt() { return 0.0; };
    void set_error() {}; // Not yet implemented for this motor type
    void master_switch();
    void maxon_manager(bool manager_active); /**< Quickly and automatically reset the Maxon motor in case of the driver board reporting an error. */

protected:
    bool _enable_response;                                                    /**< True if the motor responded to an enable command */
    bool do_scan4maxon_err_left = true;                                       /**< Part of the Maxon motor driver error reporting utilities: A switch to enable or disable error detection */
    bool maxon_counter_active_left = false;                                   /**< Part of the Maxon motor driver error reporting utilities: A switch for the error detection counter */
    unsigned long zen_millis_left;                                            /**< Part of the Maxon motor driver error reporting utilities: A timer for the motor reset function */
    bool do_scan4maxon_err_right = true;                                      /**< Part of the Maxon motor driver error reporting utilities: A switch to enable or disable error detection */
    bool maxon_counter_active_right = false;                                  /**< Part of the Maxon motor driver error reporting utilities: A switch for the error detection counter */
    unsigned long zen_millis_right;                                           /**< Part of the Maxon motor driver error reporting utilities: A timer for the motor reset function */
    const int _ctrl_left_pin = logic_micro_pins::maxon_ctrl_left_pin;         /**< Teensy pin to transmit left Maxon motor pwm signals */
    const int _ctrl_right_pin = logic_micro_pins::maxon_ctrl_right_pin;       /**< Teensy pin to transmit right Maxon motor pwm signals */
    const int _err_left_pin = logic_micro_pins::maxon_err_left_pin;           /**< Teensy pin to receive left Maxon motor driver errors */
    const int _err_right_pin = logic_micro_pins::maxon_err_right_pin;         /**< Teensy pin to receive right Maxon motor driver errors */
    const int _current_left_pin = logic_micro_pins::maxon_current_left_pin;   /**< Teensy pin to receive left Maxon motor current data */
    const int _current_right_pin = logic_micro_pins::maxon_current_right_pin; /**< Teensy pin to receive right Maxon motor current data */
    const int _pwm_neutral_val = logic_micro_pins::maxon_pwm_neutral_val;     /**< Neutral pwm command for Maxon motor drivers */
    const int _pwm_u_bound = logic_micro_pins::maxon_pwm_u_bound;             /**< Upper bound of pwm command for Maxon motor drivers */
    const int _pwm_l_bound = logic_micro_pins::maxon_pwm_l_bound;             /**< Lower bound of pwm command for Maxon motor drivers */
};

class _CANMotor : public _Motor
{
public:
    _CANMotor(config_defs::joint_id id, ExoData *exo_data, int enable_pin);
    virtual ~_CANMotor() {};
    void transaction(float torque);
    void read_data();
    void send_data(float torque);
    void on_off();
    bool enable();
    bool enable(bool overide);
    void zero();
    float get_Kt();
    void check_response();
    void set_error();

protected:
    void set_Kt(float Kt);

    // CAN통신은 메모리를 아끼기 위해 실수 데이터를 넣는 게 아니라 정수로 변환해서 보내는데, 이 함수를 사용함
    float _float_to_uint(float x, float x_min, float x_max, int bits);

    float _uint_to_float(unsigned int x_int, float x_min, float x_max, int bits);

    //_handle_read_failure(): 모터로부터 데이터를 읽는 데 실패했을 때(타임아웃 등) 호출되는 에러 처리 함수
    void _handle_read_failure();

    float _KP_MIN;                                                                  /**< Lower limit of the P gain for the motor */
    float _KP_MAX;                                                                  /**< Upper limit of the P gain for the motor */
    float _KD_MIN;                                                                  /**< Lower limit of the D gain for the motor */
    float _KD_MAX;                                                                  /**< Upper limit of the D gain for the motor */
    float _P_MAX;                                                                   /**< Max angle of the motor */
    float _I_MAX;                                                                   /**< Max current of the motor */
    float _V_MAX;                                                                   /**< Max velocity of the motor */
    bool _enable_response; /**< True if the motor responded to an enable command */ // 활성화(enable) 명령에 모터가 응답했는지 여부를 저장하는 bool 변수
    const uint32_t _timeout = 500;                                                  /**< Time to wait for response from the motor in micro-seconds */

    std::queue<float> _measured_current; /**< Queue of the measured current values */                         // 측정된 전류 값들을 순서대로 저장하는 큐(queue) 자료구조
    const int _current_queue_size = 25; /**< Size of the queue of measured current values */                  // 위 큐에 저장할 데이터의 최대 개수
    const float _variance_threshold = 0.01; /**< Threshold for the variance of the measured current values */ // 측정된 전류 값들의 분산(variance) 임계값으로, 데이터가 안정적인지 혹은 급격히 변하는지를 판단하는 데 사용
};

class AK60 : public _CANMotor
{
public:
    AK60(config_defs::joint_id id, ExoData *exo_data, int enable_pin); // Constructor: type is the motor type
    ~AK60() {};
};

class AK60_v1_1 : public _CANMotor
{
public:
    AK60_v1_1(config_defs::joint_id id, ExoData *exo_data, int enable_pin); // Constructor: type is the motor type
    ~AK60_v1_1() {};
};

class AK80 : public _CANMotor
{
public:
    AK80(config_defs::joint_id id, ExoData *exo_data, int enable_pin); // Constructor: type is the motor type
    ~AK80() {};
};

class AK70 : public _CANMotor
{
public:
    AK70(config_defs::joint_id id, ExoData *exo_data, int enable_pin); // Constructor: type is the motor type
    ~AK70() {};
};

#endif
#endif