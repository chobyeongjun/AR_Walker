#include "error_types.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

uint8_t MotorCableError::cable_error_count[4] = {0, 0, 0, 0};

#endif