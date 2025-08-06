#include "Loadcell.h"
#include "Logger.h"
// #define TORQUE_DEBUG 1        //Uncomment if you want to print debug statments to serial monitor.

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
/*
 * Constructor for the Loadcell
 * Takes in the pin to use and sets it as an analog input if it is not the NC pin.
 * NC pin is used if the joint the object is associated with is not used, or if all the available sensor slots have been taken.
 * Calibration and readings are initialized to 0
 */
Loadcell::Loadcell(unsigned int pin)
{
    this->_is_used = (pin == logic_micro_pins::not_connected_pin ? false : true);
    this->_pin = pin;
    this->_calibration = 0;
    this->_raw_reading = 0;
    this->_calibrated_reading = 0;
    this->_start_time = 0;
    this->_last_do_calibrate = false;
    this->_zero_sum = 0;
    this->_num_calibration_samples = 0;

    // logger::print("Loadcell::Loadcell : pin = ");
    // logger::print(pin);
    // logger::print("\n");

    // Configure pin if it is used
    if (this->_is_used)
    {
        pinMode(this->_pin, INPUT);
    }

#ifdef Loadcell_DEBUG
    logger::print("Loadcell :: Constructor : pin ");
    logger::print(_pin);
    logger::println(" set");
#endif
};

bool Loadcell::calibrate(bool do_calibrate)
{
#ifdef Loadcell_DEBUG
    // logger::print("Loadcell::calibrate : do_calibrate = ");
    // logger::print(do_calibrate);
    // logger::print("\n");
#endif

    if (_is_used)
    {
#ifdef Loadcell_DEBUG
        // logger::print("Loadcell::calibrate : _is_used\n");
#endif

        // Check for rising edge of do_calibrate, and reset the values
        if (do_calibrate > _last_do_calibrate)
        {
            _start_time = millis();
            _zero_sum = 0;
            _num_calibration_samples = 0;

#ifdef Loadcell_DEBUG
            logger::print("Loadcell::calibrate : Starting Cal for pin - ");
            logger::println(_pin);
            logger::print("Loadcell::calibrate : _start_time = ");
            logger::println(_start_time);
#endif
        }

        // Check if we are within the time window and need to do the calibration
        uint16_t delta = millis() - _start_time;
        if ((_cal_time >= (delta)) && do_calibrate)
        {
            float current_reading = analogRead(_pin) * Loadcell_calibration::AI_CNT_TO_V;
            _zero_sum = _zero_sum + current_reading;
            _num_calibration_samples++;

#ifdef Loadcell_DEBUG
            logger::print("Loadcell::calibrate : Continuing Cal for pin - ");
            logger::println(_pin);
            logger::print("Loadcell::calibrate : current_reading - ");
            logger::println(current_reading);
            logger::print("Loadcell::calibrate : _zero_sum - ");
            logger::println(_zero_sum);
            logger::print("Loadcell::calibrate : _num_calibration_samples - ");
            logger::println(_num_calibration_samples);
#endif
        }

        // The time window ran out so we are done, and should average the values and set the _calibration value.
        else if (do_calibrate)
        {
            if (_num_calibration_samples > 0)
            {
                _calibration = _zero_sum / (float)_num_calibration_samples;
            }

#ifdef Loadcell_DEBUG
            logger::print("Loadcell::calibrate : Loadcell Cal Done with Calibration = ");
            logger::println(_calibration);
#endif

            // logger::print("Loadcell::calibrate : Loadcell Cal Done with cal - ");
            // logger::print(_pin);
            // logger::print("\t");
            // logger::println(_calibration);

            do_calibrate = false;
        }
        else
        {
#ifdef Loadcell_DEBUG
            // logger::print("Loadcell::calibrate : Loadcell Cal Not Done\n");
#endif
        }

        _last_do_calibrate = do_calibrate;
    }
    else
    {
#ifdef Loadcell_DEBUG
        logger::print("Loadcell::calibrate : Not _is_used\n");
#endif

        do_calibrate = false;
    }

    return do_calibrate;
};

float Loadcell::read()
{
    // Reads the raw value from the Loadcell sensor
    _raw_reading = analogRead(_pin);

    // Adjusts the Loadcell sensor value based on the reading obtained during calibration
    _calibrated_reading = (((float)_raw_reading * Loadcell_calibration::AI_CNT_TO_V) - _calibration) * Loadcell_calibration::TRQ_V_TO_NM;

#ifdef Loadcell_DEBUG
    logger::print("Loadcell :: Read : pin ");
    logger::print(_pin);
    logger::print(" : Calibrated Reading = ");
    logger::println(_calibrated_reading, 12);

    logger::print("Loadcell :: Read : pin ");
    logger::print(_pin);
    logger::print(" : Raw Reading = ");
    logger::println(_raw_reading);
    logger::println(" ");

    logger::print("_calibration : ");
    logger::println(_calibration);
    logger::print("Loadcell_calibration::AI_CNT_TO_V : ");
    logger::println(Loadcell_calibration::AI_CNT_TO_V, 12);
    logger::print("Loadcell_calibration::TRQ_V_TO_NM : ");
    logger::println(Loadcell_calibration::TRQ_V_TO_NM);
#endif

    return _calibrated_reading;
};

float Loadcell::readOffset()
{
    return _calibration;
};

float Loadcell::read_microSD(float _calibration_microSD)
{
    _raw_reading = analogRead(_pin);

    // Loadcell value calculated based on the offset pulled from the SD card
    _calibrated_reading_microSD = (((float)_raw_reading * Loadcell_calibration::AI_CNT_TO_V) - _calibration_microSD) * Loadcell_calibration::TRQ_V_TO_NM;
    return _calibrated_reading_microSD;
};

#endif