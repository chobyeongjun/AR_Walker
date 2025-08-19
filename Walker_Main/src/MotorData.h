#ifndef MotorData_h
#define MotorData_h

#include "Arduino.h"

#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>

// Forward declaration
class ExoData;

class MotorData
{
public: // ParseIni.h에 정의된 config_defs::joint_id enum class를 사용하여 MotorData 클래스의 생성자에서 motor_id를 초기화
    MotorData(config_defs::joint_id id, uint8_t *config_to_send);


    void reconfigure(uint8_t *config_to_send);

    config_defs::joint_id id; 
    uint8_t motor_type;       
    float last_command;       
    float p;                  
    float v;                  
    float i;                  
    float kt;                 
    float p_des = 0;          
    float v_des = 0;          
    float kp = 0;             
    float kd = 0;             
    float t_ff = 0;           

    bool do_zero;        
    bool enabled;        
    bool is_on;          
    bool is_left;        
    bool flip_direction; 
    float gearing;       

    // Timeout state
    int timeout_count;          /**< Number of timeouts in a row */
    int timeout_count_max = 40; /**< Number of timeouts in a row before the motor is disabled */

    // Real-time Maxon Motor Reset Feedback
    int maxon_plotting_scalar = 1;
};

#endif