#ifndef LOADCELL_H
#define LOADCELL_H

#include "Arduino.h"
#include "Board.h"
#include "Logger.h"
#include "Config.h"

class Loadcell
{
public:
    Loadcell(unsigned int pin, bool is_left);
    float read();

    int readRaw();

    int _raw_reading;
    float _calibrated_weight;

private:
    bool _is_used;
    bool _is_left;
    unsigned int _pin;
};

#endif // LOADCELL_H