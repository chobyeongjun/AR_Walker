#ifndef ComsLed_h
#define ComsLed_h

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#include "Arduino.h"

class ComsLed 
{
    public:

    static ComsLed* get_instance();


    void set_color(uint8_t r, uint8_t g, uint8_t b);

    void get_color(uint8_t* r, uint8_t* g, uint8_t* b);

    void life_pulse();


    private:

    ComsLed();


    uint8_t _handle_active_low(uint8_t value);

    uint8_t r;

    uint8_t g;

    uint8_t b;

    int life_pulse_counter = 0;

    const int life_pulse_frequency = 100;
};

#endif
#endif