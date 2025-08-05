#ifndef SIDE_H
#define SIDE_H

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "Joint.h"
#include "Controller.h"
#include "ParseIni.h"
#include "Board.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include "SideData.h"
#include "ExoData.h"
#include "IMU.h"
#include "Loadcell.h"

#include <stdint.h>
#include <algorithm>

class Side
{
    public:
        Side(bool is_left, ExoData* exo_data);
        
        void run_side(); 
        void check_calibration();
        void read_data(); 
        void update_motor_cmds();   
        void set_controller(int joint, int controller);  
        void clear_step_time_estimate();
        void disable_motors();

    private:
        bool _check_imu_toe_off();

        void _update_gait_duration();

        float _calc_percent_gait();

                                     //Data that can be accessed
        ExoData* _exo_data;
        SideData* _side_data;

          //Joint objects for the side.
        _Joint _knee;
        _Joint _ankle;
        
        //IMU object for the side
        IMU _imu;
        
        bool _is_left;

               // Toe-off 감지를 위한 내부 상태 변수
        float _prev_ankle_angle = 0.0f;
        float _current_ankle_angle = 0.0f;
        float _prev_gyro_y = 0.0f;
        float _current_gyro_y = 0.0f;
        bool _was_in_plantarflexion = false;

        
        unsigned int _toe_off_timestamp;
};
#endif
#endif