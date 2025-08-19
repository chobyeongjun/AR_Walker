#ifndef Exo_h
#define Exo_h

// Arduino compiles everything in the src folder even if not included so it causes an error for the nano if this is not included
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "Side.h"
#include <stdint.h>
#include "ParseIni.h"
#include "Board.h"
#include "Utilities.h"
#include "SyncLed.h"
#include "StatusLed.h"
#include "StatusDefs.h"
#include "Config.h"
#include "ExoData.h"
#include "IMU.h"
#include "Loadcell.h"

class Exo
{
public:
    Exo(ExoData *exo_data); // Constructor: uses initializer list for the Side objects.

    bool run();

    ExoData *data;   /**< Pointer to ExoData that is getting updated by the coms mcu so they share format.*/
    Side left_side;  /**< Left side object that contains all the joints and sensors for that side */
    Side right_side; /**< Right side object that contains all the joints and sensors for that side */

#ifdef USE_SPEED_CHECK
    utils::SpeedCheck speed_check; /**< Used to check the speed of the loop without needing prints */
#endif

    SyncLed sync_led;     /**< Used to syncronize data with a motion capture system */
    StatusLed status_led; /**< Used to display the system status */

private:
    IMU _imu_left_ankle;   
    IMU _imu_right_ankle; 
    IMU _imu_left_knee;   
    IMU _imu_right_knee;   

    void _update_imu_data();
};
#endif

#endif