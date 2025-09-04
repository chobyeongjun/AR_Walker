#include "SideData.h"
#include "Logger.h"

SideData::SideData(bool is_left, uint8_t *config_to_send)
    : knee(
          (is_left ? config_defs::joint_id::left_knee : config_defs::joint_id::right_knee),
          config_to_send),
      ankle(
          (is_left ? config_defs::joint_id::left_ankle : config_defs::joint_id::right_ankle),
          config_to_send)
{
    this->is_left = is_left;

    // 보행 주기 관련 변수 초기화
    this->percent_gait = -1;
    this->expected_swing_duration = -1;
    this->ankle_angle_at_ground_strike = 0.5f;

    // Check if the side is used from the config
    if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) & this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) & !this->is_left))
    {
        this->is_used = true;
    }
    else
    {
        this->is_used = false;
    }
};

void SideData::reconfigure(uint8_t *config_to_send)
{
    if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) & this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) & !this->is_left))
    {
        this->is_used = true;
    }
    else
    {
        this->is_used = false;
    }

};

//****************************************************
// Gait State Detection Functions
//****************************************************

bool SideData::is_swing_phase() const 
{
    // Swing phase: from heel_off to next heel_strike
    // We use percent_gait which represents swing progress (0-100%)
    return heel_off && !heel_strike && percent_gait > 0;
}

bool SideData::is_stance_phase() const 
{
    // Stance phase: from heel_strike to heel_off  
    return !is_swing_phase();
}

bool SideData::is_early_swing() const 
{
    // Early swing: 0-30% of swing phase
    return is_swing_phase() && percent_gait >= 0 && percent_gait < 30;
}

bool SideData::is_mid_swing() const 
{
    // Mid swing: 30-60% of swing phase
    return is_swing_phase() && percent_gait >= 30 && percent_gait < 60;
}

bool SideData::is_late_swing() const 
{
    // Late swing: 60-100% of swing phase  
    return is_swing_phase() && percent_gait >= 60 && percent_gait <= 100;
}

bool SideData::is_early_stance() const 
{
    // Early stance: just after heel strike
    // Use a time-based approach since percent_gait resets during stance
    static unsigned long stance_start_time = 0;
    const unsigned long EARLY_STANCE_DURATION = 200; // 200ms early stance
    
    if (heel_strike && is_stance_phase()) {
        stance_start_time = millis();
    }
    
    return is_stance_phase() && (millis() - stance_start_time < EARLY_STANCE_DURATION);
}

bool SideData::is_late_stance() const 
{
    // Late stance: preparing for swing (pre-swing)
    // Detected when ankle starts to plantarflex before heel_off
    return is_stance_phase() && !is_early_stance();
}
