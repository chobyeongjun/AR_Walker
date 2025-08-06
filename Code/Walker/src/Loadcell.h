#ifndef LOADCELL_H
#define LOADCELL_H

#include "Arduino.h"
#include "Board.h"
#include "Logger.h"
#include "Config.h"

class Loadcell
{
public:
    Loadcell(unsigned int pin);

    float read(float bias, float sensitive);

    int readRaw();

private:
    bool _is_used;
    unsigned int _pin;

    int _raw_reading;
    float _calibrated_weight;
};

#endif // LOADCELL_H