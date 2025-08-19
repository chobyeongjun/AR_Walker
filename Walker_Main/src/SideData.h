#ifndef SideData_h
#define SideData_h

#include "Arduino.h"

#include "JointData.h"
#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>

// Forward declaration
class ExoData;

class SideData
{

public:
    SideData(bool is_left, uint8_t *config_to_send);

    void reconfigure(uint8_t *config_to_send);

    JointData knee;  
    JointData ankle; 
 
    float percent_gait;           /**< Estimate of the percent gait based on heel strike */
    float expected_swing_duration; /**< Estimate of how long the next swing will take based on the most recent swing times */

    bool heel_strike;
    bool heel_off;  

    bool is_left;                            /**< 1 if the side is on the left, 0 otherwise */
    bool is_used;                            /**< 1 if the side is used, 0 otherwise */
    
    float ankle_angle_at_ground_strike;         
    float expected_duration_window_upper_coeff; 
    float expected_duration_window_lower_coeff;
};

#endif