#include "SideData.h"
#include "Logger.h"

SideData::SideData(bool is_left, uint8_t* config_to_send)
: knee(
    (is_left ? config_defs::joint_id::left_knee : config_defs::joint_id::right_knee),
    config_to_send,
    is_left ? loadcell_calibration::left_knee_bias : loadcell_calibration::right_knee_bias,
    is_left ? loadcell_calibration::left_knee_sensitive : loadcell_calibration::right_knee_sensitive 
  ), 
   ankle(
    (is_left ? config_defs::joint_id::left_ankle : config_defs::joint_id::right_ankle),
    config_to_send,
    is_left ? loadcell_calibration::left_ankle_bias : loadcell_calibration::right_ankle_bias,
    is_left ? loadcell_calibration::left_ankle_sensitive : loadcell_calibration::right_ankle_sensitive 
  )
{
       this->is_left = is_left;
    
    // 보행 주기 관련 변수 초기화
    this->percent_gait = -1; 
    this->expected_swing_duration = -1;
    this->ankle_angle_at_ground_strike = 0.5f; 
    
    //Check if the side is used from the config
    if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
        || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) & this->is_left)
        || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) & !this->is_left)
       )
    {
        this->is_used = true;
    }
    else
    {
        this->is_used = false;
    }

        
};


void SideData::reconfigure(uint8_t* config_to_send) 
{
    if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
        || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) & this->is_left)
        || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) & !this->is_left)
       )
    {
        this->is_used = true;
    }
    else
    {
        this->is_used = false;
    }
    
    //Reconfigure the joints
    knee.reconfigure(config_to_send);
    ankle.reconfigure(config_to_send);


};
 