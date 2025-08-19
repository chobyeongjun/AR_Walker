#ifndef ERROR_CODES_H
#define ERROR_CODES_H

enum ErrorCodes : int
{
    NO_ERROR = 0, // This must be the first element
    TEST_ERROR,
    POOR_STATE_VARIANCE_ERROR,
    POOR_TRANSMISSION_EFFICIENCY_ERROR,
    FORCE_OUT_OF_BOUNDS_ERROR,
    FORCE_VARIANCE_ERROR,
    IMU_ERROR,
    TRACKING_ERROR,
    MOTOR_TIMEOUT_ERROR,

    ERROR_CODE_LENGTH // This must be the last element
};

#endif // ERROR_CODES_H