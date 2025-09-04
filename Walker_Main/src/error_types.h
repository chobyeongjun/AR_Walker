#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "JointData.h"
#include "error_codes.h"
#include "Utilities.h"
#include "Logger.h"

// Forward declaration for ExoData
class ExoData;

// Create abstract class for joint-level error types
class ErrorType
{
public:
    ErrorType() {};
    virtual bool check(JointData *_data) = 0;  // 모든 에러는 자신이 발생했는지 **검사(check)**하는 방법을 반드시 가져야 한다
    virtual void handle(JointData *_data) = 0; // "모든 에러는 발생했을 때 어떻게 **대처(handle)**할지 방법을 반드시 가져야 한다
};


class TestError : public ErrorType
{
public:
    TestError() : ErrorType() {};

    bool check(JointData *_data)
    {
        // return millis() > 45000;
        return false;
    }
    void handle(JointData *_data)
    {
        _data->motor.enabled = false;
        logger::println("Test Error", LogLevel::Error);
    }
};

class PoorStateVarianceError : public ErrorType
{
public:
    PoorStateVarianceError() : ErrorType() {};

    bool check(JointData *_data)
    {

        return false;
    }
    void handle(JointData *_data)
    {
    }
};

class PoorTransmissionEfficiencyError : public ErrorType
{
public:
    PoorTransmissionEfficiencyError() : ErrorType() {};

    bool check(JointData *_data)
    {
        return false; // abs(torque_error) > 100 * (1 - _data->transmission_efficiency_threshold); //This should be the normal method for handling this error but we encountered issues so currently set to do nothing
    }
    void handle(JointData *_data)
    {
        //_data->motor.enabled = false;
        logger::println("Transmission Efficiency Error", LogLevel::Error);
    }
};

class ForceOutOfBoundsError : public ErrorType
{
public:
    ForceOutOfBoundsError() : ErrorType() {};

    bool check(JointData *_data)
    {
        return false;
    }
    void handle(JointData *_data)
    {
        //_data->motor.enabled = false;
        logger::println("Force Out of Bounds Error", LogLevel::Error);
    }
};

class ForceVarianceError : public ErrorType
{
public:
    ForceVarianceError() : ErrorType() {};

    bool check(JointData *_data)
    {

        // If the failure count is greater than the max, return true
        return false;
    }
    void handle(JointData *_data)
    {
        //_data->motor.enabled = false;
        logger::println("Force Variance Error", LogLevel::Error);
    }
};

class TrackingError : public ErrorType
{
public:
    TrackingError() : ErrorType() {};

    bool check(JointData *_data)
    {
        return false;
    }
    void handle(JointData *_data)
    {
    }
};

class MotorTimeoutError : public ErrorType
{
public:
    MotorTimeoutError() : ErrorType() {};

    bool check(JointData *_data)
    {
        const bool timeout_error = _data->motor.timeout_count >= _data->motor.timeout_count_max;
        if (timeout_error)
        {
            _data->motor.timeout_count = 0;
        }
        return timeout_error;
    }
    void handle(JointData *_data)
    {
        //_data->motor.enabled = false;
        logger::println("Motor Timeout Error", LogLevel::Error);
    }
};

class IMUBatteryLowError : public ErrorType
{
public:
    IMUBatteryLowError() : ErrorType() {};

    bool check(JointData *_data)
    {
        return (_data->imu_battery <= 5);
    }
    void handle(JointData *_data)
    {
        _data->motor.enabled = false;
        logger::println("IMU Battery Critical Low Error - Joint Disabled", LogLevel::Error);
    }
};

class IMUBatteryWarning : public ErrorType
{
public:
    IMUBatteryWarning() : ErrorType() {};

    bool check(JointData *_data)
    {
        return (_data->imu_battery <= 10 && _data->imu_battery > 5);
    }
    void handle(JointData *_data)
    {
        logger::println("IMU Battery Warning - Replace Soon", LogLevel::Warn);
    }
};

class MotorTemperatureError : public ErrorType
{
public:
    MotorTemperatureError() : ErrorType() {};

    bool check(JointData *_data)
    {
        return (_data->motor.temperature >= 110);
    }
    void handle(JointData *_data)
    {
        _data->motor.enabled = false;
        logger::println("Motor Temperature Critical Error - Joint Disabled", LogLevel::Error);
    }
};

class MotorTemperatureWarning : public ErrorType
{
public:
    MotorTemperatureWarning() : ErrorType() {};

    bool check(JointData *_data)
    {
        return (_data->motor.temperature >= 100 && _data->motor.temperature < 110);
    }
    void handle(JointData *_data)
    {
        logger::println("Motor Temperature Warning - Monitor Closely", LogLevel::Warn);
    }
};

class MotorCurrentOverloadError : public ErrorType
{
public:
    MotorCurrentOverloadError() : ErrorType() {};

    bool check(JointData *_data)
    {
        return (abs(_data->motor.i) >= 13.5);
    }
    void handle(JointData *_data)
    {
        _data->motor.enabled = false;
        logger::println("Motor Current Overload Error - Joint Disabled", LogLevel::Error);
    }
};

class MotorCableError : public ErrorType
{
private:
    static uint8_t cable_error_count[4];

public:
    MotorCableError() : ErrorType() {};

    bool check(JointData *_data)
    {
        uint8_t joint_index = static_cast<uint8_t>(_data->id);
        
        if (abs(_data->motor.i) >= 7.0 && abs(_data->loadcell_reading) < 5.0)
        {
            cable_error_count[joint_index]++;
            if (cable_error_count[joint_index] >= 10)
            {
                cable_error_count[joint_index] = 0;
                return true;
            }
        }
        else
        {
            if (cable_error_count[joint_index] > 0)
            {
                cable_error_count[joint_index]--;
            }
        }
        return false;
    }
    void handle(JointData *_data)
    {
        _data->motor.enabled = false;
        logger::println("Motor Cable Error - Check Connections - Joint Disabled", LogLevel::Error);
    }
};

class LipoBatteryError : public ErrorType
{
public:
    LipoBatteryError() : ErrorType() {};

    bool check(JointData *_data)
    {
        if (_data->parent_exo == nullptr) return false;
        
        const float V_warning = 19.2f;  // 경고 수준 (3.2V × 6셀)
        return (_data->parent_exo->battery_percent <= 15.0f || _data->parent_exo->battery_value <= V_warning);
    }
    void handle(JointData *_data)
    {
        if (_data->parent_exo == nullptr) return;
        
        // 모든 관절 비활성화
        _data->parent_exo->left_side.ankle.motor.enabled = false;
        _data->parent_exo->left_side.knee.motor.enabled = false; 
        _data->parent_exo->right_side.ankle.motor.enabled = false;
        _data->parent_exo->right_side.knee.motor.enabled = false;
        
        logger::println("LiPo Battery Critical Low - All Motors Disabled", LogLevel::Error);
    }
};

class LipoBatteryWarning : public ErrorType
{
public:
    LipoBatteryWarning() : ErrorType() {};

    bool check(JointData *_data)
    {
        if (_data->parent_exo == nullptr) return false;
        
        return (_data->parent_exo->battery_percent <= 25.0f && _data->parent_exo->battery_percent > 15.0f);
    }
    void handle(JointData *_data)
    {
        logger::println("LiPo Battery Warning - Charge Soon", LogLevel::Warn);
    }
};

#endif
#endif // ERROR_TYPES_H