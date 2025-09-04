#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "JointData.h"
#include "error_codes.h"
#include "Utilities.h"
#include "Logger.h"
#include <queue>
#include <cmath>
#include <cstdlib>    // abs() 함수용
#include <algorithm>  // max(), min() 함수용

// Forward declaration
class ExoData;

// Create abstract class for error types
class ErrorType
{
public:
    ErrorType() {};
    virtual bool check(JointData *_data, ExoData *exo_data) = 0;  // 모든 에러는 자신이 발생했는지 **검사(check)**하는 방법을 반드시 가져야 한다
    virtual void handle(JointData *_data, ExoData *exo_data) = 0; // 모든 에러는 발생했을 때 어떻게 **대처(handle)**할지 방법을 반드시 가져야 한다
};

class TestError : public ErrorType
{
public:
    TestError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        // return millis() > 45000;
        return false;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        _data->motor.is_on = false;
        logger::println("Test Error", LogLevel::Error);
    }
};


class IMUError : public ErrorType
{
public:
    IMUError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
    
        if (_data->imu_time_error_detected)
        {
            logger::println("IMU Error - Time continuity error detected", LogLevel::Error);
            return true;
        }
        
        // 2. NaN 값 확인
        if (isnan(_data->imu_pitch) || isnan(_data->imu_gyro_y))
        {
            logger::println("IMU Error - NaN detected in sensor data", LogLevel::Error);
            return true;
        }
        
        // 3. 비정상적인 범위 값 확인
        if (abs(_data->imu_pitch) > 150 || abs(_data->imu_gyro_y) > 5000)
        {
            logger::println("IMU Error - Sensor data out of range", LogLevel::Error);
            return true;
        }
        
        // 4. 센서 데이터가 일정 시간 동안 변화하지 않는 경우 (센서 고착)
        static float prev_pitch = 0;
        static float prev_gyro_y = 0;
        static int unchanged_count = 0;
        
        const float EPSILON = 0.0001f;  // Float 비교 허용 오차
        if (abs(_data->imu_pitch - prev_pitch) < EPSILON && abs(_data->imu_gyro_y - prev_gyro_y) < EPSILON)
        {
            if (unchanged_count < 32000) {  // Overflow 방지
                unchanged_count++;
            }
            if (unchanged_count > 100)  // 100회 연속 동일한 값 (약 0.1초 at 1kHz)
            {
                logger::println("IMU Error - Sensor data frozen", LogLevel::Error);
                return true;
            }
        }
        else
        {
            unchanged_count = 0;
        }
        
        prev_pitch = _data->imu_pitch;
        prev_gyro_y = _data->imu_gyro_y;
        
        return false;
    }
    
    void handle(JointData *_data, ExoData *exo_data)
    {
        // IMU 에러 발생 시 안전한 기본값 설정
        _data->imu_pitch = 0.0f;
        _data->imu_gyro_y = 0.0f;
        
        // 부드러운 브레이크로 모터 보조 일시 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        
        logger::println("IMU Error - 1A BRAKE APPLIED for safety", LogLevel::Error);
    }
};


class LoadcellSensorError : public ErrorType
{
public:
    LoadcellSensorError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        float force_reading = _data->loadcell_reading;
        
        // 1. 물리적 불가능한 값 (인간이 낼 수 없는 힘)
        if (abs(force_reading) > 200.0f) {  // 200N 초과
            return true;
        }
        
        // 2. 센서 고착 상태 - 사용자 피드백 반영: 1000번 = 1초간 0.1N 이상 변화 없음
        static float prev_reading = 0;
        static int unchanged_count = 0;
        
        if (abs(force_reading - prev_reading) < 0.1f) {  // 0.1N 차이보다 작으면
            if (unchanged_count < 32000) {  // Overflow 방지
                unchanged_count++;                        // 카운트 증가
            }
            if (unchanged_count > 1000) {                // 1000번 연속(1초) 거의 안 변함
                return true;
            }
        } else {
            unchanged_count = 0;
        }
        prev_reading = force_reading;
        
        return false;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        logger::println("Loadcell Sensor Error - Check sensor connection", LogLevel::Error);
    }
};


class EmergencyStopError : public ErrorType
{
public:
    EmergencyStopError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        // ExoData의 estop 플래그 확인 - 이제 직접 접근 가능
        return exo_data->estop;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // Emergency Stop: 부드러운 브레이크로 안전한 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("EMERGENCY STOP ACTIVATED - 1A BRAKE APPLIED", LogLevel::Error);
    }
};


class ForceSensorRangeError : public ErrorType
{
public:
    ForceSensorRangeError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        float measured_force = _data->loadcell_reading;
        
        // 절대적 힘 한계 (안전을 위해)
        if (abs(measured_force) > 160.0f) {  // 160N 초과는 위험
            return true;
        }
        
        return false;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // 힘 센서 과부하: 부드러운 브레이크 적용
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("Force Sensor Range Error - 1A BRAKE APPLIED", LogLevel::Error);
    }
};


class BatteryLowError : public ErrorType
{
public:
    BatteryLowError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        // 임시로 무조건 통과하도록 설정 (나중에 배터리 전압 로직 구현 예정)
        return exo_data->battery_value < -1.0f;  // 항상 false가 되도록 설정
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        _data->motor.is_on = false;
        logger::println("Battery Low - System shutdown for safety", LogLevel::Error);
    }
};


class MotorCurrentOverloadError : public ErrorType
{
private:
    // 스톨 감지를 위한 전류 분석 변수들 (Motor.cpp와 동일한 로직)
    float _measured_current[10];  // 큐 대신 배열 사용 (메모리 최적화)
    uint8_t _current_index = 0;   // 현재 인덱스
    bool _buffer_full = false;    // 버퍼가 가득 찼는지 여부
    const size_t _current_queue_size = 10;  // 배열 크기 (Motor.cpp 참조)
    const float _variance_threshold = 0.1f;  // 분산 임계값 (Motor.cpp 참조)

public:
    MotorCurrentOverloadError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        float motor_current = _data->motor.i;  // 실제 전류
        
        // 1. 모터 전류가 정격 초과 (AK 모터 한계 기준) - 기존 로직
        if (abs(motor_current) > 13.5f) {  // 13.5A 초과는 위험
            logger::println("Motor Current Overload - Excessive current detected", LogLevel::Error);
            return true;
        }
        
        // 2. 모터 스톨 감지 (Motor.cpp check_response() 로직 적용)
        // 배열에 새 데이터 추가 (원형 버퍼 방식)
        _measured_current[_current_index] = motor_current;
        _current_index = (_current_index + 1) % _current_queue_size;
        if (_current_index == 0) _buffer_full = true;  // 한 바퀴 돌면 가득 참
        
        // 충분한 데이터가 있을 때만 분석
        if (_buffer_full || _current_index >= _current_queue_size)
        {
            // 전류 분산 계산 
            float sum = 0.0f, sum_sq = 0.0f;
            size_t data_count = _buffer_full ? _current_queue_size : _current_index;
            
            for (size_t i = 0; i < data_count; i++) {
                float val = _measured_current[i];
                sum += val;
                sum_sq += val * val;
            }
            
            float mean = sum / data_count;
            float variance = (sum_sq / data_count) - (mean * mean);
            
            // 전류 분산이 너무 낮으면 스톨 의심
            if (variance < _variance_threshold && abs(mean) > 1.0f) {  // 평균 전류가 1A 이상일 때만
                logger::println("Motor Current Overload - Motor stall detected (low current variance)", LogLevel::Error);
                return true;
            }
        }
        
        return false;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // 모터 전류 과부하: 부드러운 브레이크로 안전 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("Motor Current Overload - 1A BRAKE APPLIED for safety", LogLevel::Error);
    }
};


class MotorTimeoutError : public ErrorType
{
public:
    MotorTimeoutError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        // CAN 버스를 통한 모터 통신 실패 감지
        // timeout_count는 연속된 통신 실패 횟수를 나타냄
        const bool timeout_error = _data->motor.timeout_count >= _data->motor.timeout_count_max;
        if (timeout_error)
        {
            _data->motor.timeout_count = 0;
        }
        return timeout_error;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // 모터 통신 실패: 부드러운 브레이크로 안전 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("Motor Communication Error - 1A BRAKE APPLIED", LogLevel::Error);
    }
};


class MotorTemperatureError : public ErrorType
{
public:
    MotorTemperatureError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        return (_data->motor.temperature >= 110);
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // 모터 과열: 부드러운 브레이크로 안전 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("Motor Temperature Critical Error - 1A BRAKE APPLIED", LogLevel::Error);
    }
};

class MotorTemperatureWarning : public ErrorType
{
public:
    MotorTemperatureWarning() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        return (_data->motor.temperature >= 100 && _data->motor.temperature < 110);
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        logger::println("Motor Temperature Warning - Monitor Closely", LogLevel::Warn);
    }
};

class IMUBatteryLowError : public ErrorType
{
public:
    IMUBatteryLowError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        return (_data->imu_battery <= 5);
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // IMU 배터리 부족: 부드러운 브레이크로 안전 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("IMU Battery Critical Low Error - 1A BRAKE APPLIED", LogLevel::Error);
    }
};

class IMUBatteryWarning : public ErrorType
{
public:
    IMUBatteryWarning() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        return (_data->imu_battery <= 10 && _data->imu_battery > 5);
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        logger::println("IMU Battery Warning - Replace Soon", LogLevel::Warn);
    }
};

class MotorCableError : public ErrorType
{
private:
    // inline static: 헤더에서 바로 정의, 별도 cpp 파일 불필요
    static inline uint8_t cable_error_count[4] = {0, 0, 0, 0};

public:
    MotorCableError() : ErrorType() {};

    bool check(JointData *_data, ExoData *exo_data)
    {
        // joint_id enum 값을 0~3 인덱스로 변환
        uint8_t joint_index = 0;
        switch (_data->id) {
            case config_defs::joint_id::left_knee:   joint_index = 0; break;
            case config_defs::joint_id::right_knee:  joint_index = 1; break;
            case config_defs::joint_id::left_ankle:  joint_index = 2; break;
            case config_defs::joint_id::right_ankle: joint_index = 3; break;
            default: return false; // 알 수 없는 joint_id
        }
        
        // 7A 이상 전류 + 5N 미만 로드셀이 감지되면
        if (abs(_data->motor.i) >= 7.0 && _data->loadcell_reading < 5.0)
        {
            cable_error_count[joint_index]++;
            // 20회 연속으로 감지되면 진짜 케이블 문제
            if (cable_error_count[joint_index] >= 20)
            {
                cable_error_count[joint_index] = 0; // 카운터 리셋
                return true;
            }
        }
        else
        {
            // 정상 상태면 카운터 감소 (점진적 회복)
            if (cable_error_count[joint_index] > 0)
            {
                cable_error_count[joint_index]--;
            }
        }
        return false;
    }
    void handle(JointData *_data, ExoData *exo_data)
    {
        // 모터 케이블 오류: 부드러운 브레이크로 안전 정지
        _data->brake_requested = true;
        _data->brake_current = 1.0f;  // 1A 부드러운 브레이크
        _data->motor.is_on = false;
        logger::println("Motor Cable Error - 1A BRAKE APPLIED, Check Connections", LogLevel::Error);
    }
};

#endif
#endif // ERROR_TYPES_H