#ifndef JointData_h
#define JointData_h

#include "Arduino.h"

#include "MotorData.h"
#include "ControllerData.h"
#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>
#include <queue>

// Forward declaration
class ExoData;

class JointData
{
public:
    JointData(config_defs::joint_id id, uint8_t* config_to_send);

    void reconfigure(uint8_t *config_to_send);

    config_defs::joint_id id;    
    MotorData motor;             
    ControllerData controller;    

    bool is_left;                 
    bool is_used;                 

    float position;               
    float velocity;              

    float loadcell_reading;
    float imu_pitch  ;
    float imu_gyro_y ;
    uint16_t imu_battery;
    
    ExoData *parent_exo; /**< Pointer to parent ExoData for system-level access */
};

#endif