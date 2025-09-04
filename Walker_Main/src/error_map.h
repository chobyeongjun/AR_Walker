#ifndef ERROR_MAPS_H
#define ERROR_MAPS_H
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "error_codes.h"
#include "error_types.h"
#include <map>

const std::map<ErrorCodes, ErrorType *> error_map = {
    {TEST_ERROR, new TestError()},
    {POOR_STATE_VARIANCE_ERROR, new PoorStateVarianceError()},
    {POOR_TRANSMISSION_EFFICIENCY_ERROR, new PoorTransmissionEfficiencyError()},
    {FORCE_OUT_OF_BOUNDS_ERROR, new ForceOutOfBoundsError()},
    {FORCE_VARIANCE_ERROR, new ForceVarianceError()},
    {IMU_ERROR, new ForceVarianceError()},
    {TRACKING_ERROR, new TrackingError()},
    {MOTOR_TIMEOUT_ERROR, new MotorTimeoutError()},
    {IMU_BATTERY_LOW_ERROR, new IMUBatteryLowError()},
    {IMU_BATTERY_WARNING, new IMUBatteryWarning()},
    {MOTOR_TEMPERATURE_ERROR, new MotorTemperatureError()},
    {MOTOR_TEMPERATURE_WARNING, new MotorTemperatureWarning()},
    {MOTOR_CURRENT_OVERLOAD_ERROR, new MotorCurrentOverloadError()},
    {MOTOR_CABLE_ERROR, new MotorCableError()},
    {LIPO_BATTERY_ERROR, new LipoBatteryError()},
    {LIPO_BATTERY_WARNING, new LipoBatteryWarning()}};

#endif
#endif // ERROR_MAPS_H