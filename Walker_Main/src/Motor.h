#ifndef Motor_h
#define Motor_h

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"
#include "ExoData.h"
#include <cstdint>
#include <queue>
#include "FlexCAN_T4.h"

// 서보 모드용 CAN 명령어 ID
enum class CAN_PACKET_ID : uint8_t {
    SET_DUTY = 0,
    SET_CURRENT = 1,
    SET_CURRENT_BRAKE = 2,
    SET_RPM = 3,
    SET_POS = 4,
    SET_ORIGIN_HERE = 5,
    SET_POS_SPD = 6
};

// 서보 모드 에러 코드 정의
enum class MotorErrorCode : uint8_t {
    NO_FAULT = 0,
    MOTOR_OVER_TEMPERATURE = 1,
    OVER_CURRENT = 2,
    OVER_VOLTAGE = 3,
    UNDER_VOLTAGE = 4,
    ENCODER_FAULT = 5,
    MOSFET_OVER_TEMPERATURE = 6,
    MOTOR_STALL = 7
};

// 에러 코드를 문자열로 변환하는 헬퍼 함수 선언
const char* motorErrorCodeToString(uint8_t code);

class _Motor
{
public:
    _Motor(config_defs::joint_id id, ExoData *exo_data);
    virtual ~_Motor() {};

    virtual void read_data() = 0;
    virtual void set_position(float position) = 0;
    virtual void set_speed(float rpm) = 0;
    virtual void set_position_speed(float pos, float spd, float acc) = 0;
    virtual void on_off() = 0;
    virtual void zero() = 0;
    virtual float get_Kt() = 0;
    virtual void set_error() = 0; 

    
    virtual bool get_is_left();
    virtual config_defs::joint_id get_id();

    bool _is_on;

protected:
    config_defs::joint_id _id;
    bool _is_left;
   
    ExoData *_data;
    MotorData *_motor_data;
    bool _error = false;
    float _Kt;
};

class NullMotor : public _Motor
{
public:
    NullMotor(config_defs::joint_id id, ExoData *exo_data) : _Motor(id, exo_data) {};
    void read_data() {};
    void set_position(float pos) {};
    void set_speed(float rpm)  {};
    void set_position_speed(float pos, float spd, float acc) {};
    void on_off() {};
    void zero() {};
    float get_Kt() { return 0.0; };
    void set_error() {};
};

class _CANMotor : public _Motor
{
public:
    _CANMotor(config_defs::joint_id id, ExoData *exo_data);
    virtual ~_CANMotor() {};
    
    void set_position(float pos);
    void set_speed(float rpm);
    void set_position_speed(float pos, float spd, float acc);
    void read_data();
    void on_off();
    void zero();
    float get_Kt();
    void check_response();
    void set_error();

protected:
    void set_Kt(float Kt);
    void _handle_read_failure();
                                                                 
    // Safety Limits
    float _P_MAX;
    float _P_MIN;
    float _V_MAX;
    float _V_MIN;
    float _A_MAX;
    float _A_MIN;
    float _I_MAX;
    float _I_MIN;

    const uint32_t _timeout = 500;

    std::queue<float> _measured_current;
    const int _current_queue_size = 25;
    const float _variance_threshold = 0.01;
};

class AK10 : public _CANMotor
{
public:
    AK10(config_defs::joint_id id, ExoData *exo_data);
    ~AK10() {};
};

class AK60_v1_1 : public _CANMotor
{
public:
    AK60_v1_1(config_defs::joint_id id, ExoData *exo_data);
    ~AK60_v1_1() {};
};

class AK80 : public _CANMotor
{
public:
    AK80(config_defs::joint_id id, ExoData *exo_data);
    ~AK80() {};
};

class AK70 : public _CANMotor
{
public:
    AK70(config_defs::joint_id id, ExoData *exo_data);
    ~AK70() {};
};

#endif
#endif