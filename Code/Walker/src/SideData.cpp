#include "SideData.h"
#include "Logger.h"
#include <string.h>

SideData::SideData(bool is_left, uint8_t *config_to_send)
    : knee(is_left, config_to_send), ankle(is_left, config_to_send)
{
    this->is_left = is_left;
    this->is_used = false;

    // GCP 관련 변수 초기화
    percent_gait = 0.0f;
    toe_off_timestamp = 0;
    expected_gait_duration = 0.0f;

    // config.ini에서 ewma_alpha 읽어오기
    this->ewma_alpha = config_to_send[config_defs::ewma_alpha_idx];

    // Loadcell 관련 변수 초기화
    loadcell_reading = 0.0f;
    this->loadcell_bias_knee = is_left ? config_to_send[config_defs::left_knee_bias_idx] : config_to_send[config_defs::right_knee_bias_idx];
    this->loadcell_sensitive_knee = is_left ? config_to_send[config_defs::left_knee_sensitive_idx] : config_to_send[config_defs::right_knee_sensitive_idx];
    this->loadcell_bias_ankle = is_left ? config_to_send[config_defs::left_ankle_bias_idx] : config_to_send[config_defs::right_ankle_bias_idx];
    this->loadcell_sensitive_ankle = is_left ? config_to_send[config_defs::left_ankle_sensitive_idx] : config_to_send[config_defs::right_ankle_sensitive_idx];

    // IMU 관련 변수 초기화
    imu_dist_x = 0;
    imu_dist_y = 0;
    imu_dist_z = 0;
    imu_gyro_x = 0;
    imu_gyro_y = 0;
    imu_gyro_z = 0;
    imu_roll = 0;
    imu_pitch = 0;
    imu_yaw = 0;

    // config.ini에서 IMU ID 읽어오기
    this->imu_id_knee = is_left ? config_to_send[config_defs::left_knee_IMU_ID_idx] : config_to_send[config_defs::right_knee_IMU_ID_idx];
    this->imu_id_ankle = is_left ? config_to_send[config_defs::left_ankle_IMU_ID_idx] : config_to_send[config_defs::right_ankle_IMU_ID_idx];

    // config.ini에서 사용 여부 읽어와 joint is_used 설정
    if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left))
    {
        is_used = true;
    }
};

void SideData::reconfigure(uint8_t *config_to_send)
{
    if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left))
    {
        is_used = true;
    }
    else
    {
        is_used = false;
    }

    // Joint reconfigure
    knee.reconfigure(config_to_send);
    ankle.reconfigure(config_to_send);
};