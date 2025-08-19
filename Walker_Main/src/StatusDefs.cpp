#include "StatusDefs.h"
#include "Logger.h"

void print_status_message(uint16_t message)
{
    switch (message)
    {
    case status_defs::messages::off:
        logger::print("Off");
        break;
    case status_defs::messages::trial_off:
        logger::print("Trial Off");
        break;
    case status_defs::messages::trial_on:
        logger::print("Trial On");
        break;
    case status_defs::messages::test:
        logger::print("Test");
        break;
    case status_defs::messages::motor_start_up:
        logger::print("Motor Start Up");
        break;
    case status_defs::messages::error:
        logger::print("General Error");
        break;
    case status_defs::messages::error_left_knee_loadcell:
        logger::print("Loadcell Error");
        break;
    case status_defs::messages::error_right_knee_loadcell:
        logger::print("Loadcell Error");
        break;
    case status_defs::messages::error_left_ankle_loadcell:
        logger::print("Loadcell Error");
        break;
    case status_defs::messages::error_right_ankle_loadcell:
        logger::print("Loadcell Error");
        break;
    case status_defs::messages::error_left_knee_imu:
        logger::print("IMU Error");
        break;
    case status_defs::messages::error_right_knee_imu:
        logger::print("IMU Error");
        break;
    case status_defs::messages::error_left_ankle_imu:
        logger::print("IMU Error");
        break;
    case status_defs::messages::error_right_ankle_imu:
        logger::print("IMU Error");
        break;
    case status_defs::messages::error_left_knee_motor:
        logger::print("Error :: Left Knee Motor");
        break;
    case status_defs::messages::error_left_ankle_motor:
        logger::print("Error :: Left Ankle Motor");
        break;
    case status_defs::messages::error_right_knee_motor:
        logger::print("Error :: Right Knee Motor");
        break;
    case status_defs::messages::error_right_ankle_motor:
        logger::print("Error :: Right Ankle Motor");
        break;
    case status_defs::messages::error_left_knee_controller:
        logger::print("Error :: Left Knee Controller");
        break;
    case status_defs::messages::error_left_ankle_controller:
        logger::print("Error :: Left Ankle Controller");
        break;
    case status_defs::messages::error_right_knee_controller:
        logger::print("Error :: Right Knee Controller");
        break;
    case status_defs::messages::error_right_ankle_controller:
        logger::print("Error :: Right Ankle Controller");
        break;
    }
};
