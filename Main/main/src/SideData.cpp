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
    this->expected_gait_duration = -1;

    this->ankle_angle_at_ground_strike = -1;
    this->heel_strike = false;
    this->heel_off = false;
    
    this->pitch_threshold = -1 ;               // 기본 5도 임계값
    this->prev_gyro_y = -1 ;                   // 이전 gyro 값 초기화
    
    // 타이밍 변수 초기화
    this->heel_off_timestamp = -1;
    this->heel_strike_timestamp = -1;
    this->prev_gait_cycle_time = -1;


    this->onset_event = false;
    this->peak_event = false;
    this->release_event = false;

    // 기본 타이밍 값 설정 (GUI에서 나중에 변경 가능)
    this->onset_timing_percent = 15.0f;   // 보행주기 15%에서 시작
    this->peak_timing_percent = 50.0f;    // 보행주기 50%에서 최대
    this->release_timing_percent = 85.0f; // 보행주기 85%에서 해제 

    // 윈도우 계수 초기화
    this->expected_duration_window_upper_coeff = 1.75;
    this->expected_duration_window_lower_coeff = 0.25;
    
    // 보조력 관련 변수 초기화
    this->max_reference_force = 20.0f;        // 기본 최대 보조력 20N
    this->current_reference_force = 0.0f;     // 현재 출력 힘 0N로 시작
    this->assistance_active = false;          // 보조 비활성 상태로 시작


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
        //Reconfigure the joints

    knee.reconfigure(config_to_send);
    ankle.reconfigure(config_to_send);

};