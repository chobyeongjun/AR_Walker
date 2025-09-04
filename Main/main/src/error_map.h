#ifndef ERROR_MAP_H
#define ERROR_MAP_H
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "error_codes.h"
#include "error_types.h"
#include <map>

// inline static 객체들 - 전체 프로그램에서 하나씩만 생성됨
inline static TestError test_error_instance;
inline static MotorTimeoutError motor_timeout_error_instance;
inline static IMUError imu_error_instance;
inline static LoadcellSensorError loadcell_sensor_error_instance;
inline static EmergencyStopError emergency_stop_error_instance;
inline static ForceSensorRangeError force_sensor_range_error_instance;
inline static BatteryLowError battery_low_error_instance;
inline static MotorCurrentOverloadError motor_current_overload_error_instance;
inline static MotorTemperatureError motor_temperature_error_instance;
inline static MotorTemperatureWarning motor_temperature_warning_instance;
inline static IMUBatteryLowError imu_battery_low_error_instance;
inline static IMUBatteryWarning imu_battery_warning_instance;
inline static MotorCableError motor_cable_error_instance; 

inline const std::map<ErrorCodes, ErrorType *> error_map = {
    {TEST_ERROR, &test_error_instance},
    {MOTOR_TIMEOUT_ERROR, &motor_timeout_error_instance},
    {IMU_ERROR, &imu_error_instance},
    {LOADCELL_SENSOR_ERROR, &loadcell_sensor_error_instance},
    {EMERGENCY_STOP_ERROR, &emergency_stop_error_instance},
    {FORCE_SENSOR_RANGE_ERROR, &force_sensor_range_error_instance},
    {BATTERY_LOW_ERROR, &battery_low_error_instance},
    {MOTOR_CURRENT_OVERLOAD_ERROR, &motor_current_overload_error_instance},
    {MOTOR_TEMPERATURE_ERROR, &motor_temperature_error_instance},
    {MOTOR_TEMPERATURE_WARNING, &motor_temperature_warning_instance},
    {IMU_BATTERY_LOW_ERROR, &imu_battery_low_error_instance},
    {IMU_BATTERY_WARNING, &imu_battery_warning_instance},
    {MOTOR_CABLE_ERROR, &motor_cable_error_instance}};

#endif
#endif // ERROR_MAP_H