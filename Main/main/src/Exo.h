#ifndef Exo_h
#define Exo_h

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

    ExoData *data;  
    Side left_side;  
    Side right_side; 

    SyncLed sync_led;     
    StatusLed status_led; 
    
    bool is_collecting_data = false;

private:
    IMU left_ankle_imu;   
    IMU right_ankle_imu; 
    IMU left_knee_imu;   
    IMU right_knee_imu;

    void _update_imu_data();

};
#endif

#endif