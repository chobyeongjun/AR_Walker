#include "Loadcell.h"
#include "Logger.h"

Loadcell::Loadcell(unsigned int pin)
{
    this->_is_used = (pin == logic_micro_pins::not_connected_pin ? false : true);
    this->_pin = pin;

    _raw_reading = 0;
    _calibrated_weight = 0;

    if (this->_is_used)
    {
        pinMode(this->_pin, INPUT);
    }
};

float Loadcell::read(float bias, float sensitive)
{
    if (_is_used)
    {
        _raw_reading = analogRead(_pin);
        float current_voltage = (float)_raw_reading * loadcell_calibration::AI_CNT_TO_V;
        _calibrated_weight = (current_voltage * sensitive + bias);
        return _calibrated_weight;
    }
    return 0.0f;
};

int Loadcell::readRaw()
{
    if (_is_used)
    {
        return analogRead(_pin);
    }
    return 0;
};