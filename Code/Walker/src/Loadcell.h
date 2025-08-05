#ifndef Loadcell_h
#define Loadcell_h

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Board.h"
#include "Arduino.h"
#include "Config.h"

enum class CalibrationState
{
    NOT_STARTED,
    ZERO_CALIBRATING,
    SPAN_CALIBRATING,
    CALIBRATED
};

class Loadcell
{
public:
    Loadcell(unsigned int pin, float reference_weight);

    bool calibrate(CalibrationState state_to_run);

    float read();

    float read_with_offset_and_sensitivity(float zero_offset, float sensitivity);

    CalibrationState getCalibrationState() { return _state; }

    float getZeroOffset() { return _zero_offset_voltage; }

    float getSensitivity() { return _sensitivity; }

private:
    unsigned int _pin; /**< Pin to read for the sensor */
    bool _is_used;     /**< Flag indicating if the sensor is used */

    float _zero_offset_voltage;
    float _span_voltage;
    float _reference_weight;
    float _sensitivity;

    float _raw_reading;
    float _calculated_weight;

    CalibrationState _state;
    unsigned long _start_time;
    float _calibration_sum;
    int _num_calibration_samples;
    /
};

#endif
