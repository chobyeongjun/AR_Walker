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
    this->loadcell_bias_knee = is_left ? config_to_send [confi #include "SideData.h"
#include "Logger.h"
                                                         /*
                                                          * Constructor for the side data.
                                                          * Takes if the side is left, and the array from the INI parser.
                                                          * Stores if the side is left, the percent gait, FSR data, and if the side is used.
                                                          * Uses an initializer list for the joint data, and creates the joint id for the joints.
                                                          */
                                                         SideData::SideData(bool is_left, uint8_t *config_to_send):hip((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::hip), config_to_send, is_left ? config_to_send[config_defs::left_hip_RoM_idx] : config_to_send[config_defs::right_hip_RoM_idx], config_to_send[config_defs::hip_flip_angle_dir_idx], is_left ? config_to_send[config_defs::left_hip_torque_offset_idx] : config_to_send[config_defs::right_hip_torque_offset_idx]),
                                                         knee((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::knee), config_to_send, is_left ? config_to_send[config_defs::left_knee_RoM_idx] : config_to_send[config_defs::right_knee_RoM_idx], config_to_send[config_defs::knee_flip_angle_dir_idx], is_left ? config_to_send[config_defs::left_knee_torque_offset_idx] : config_to_send[config_defs::right_knee_torque_offset_idx]), ankle((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::ankle), config_to_send, is_left ? config_to_send[config_defs::left_ankle_RoM_idx] : config_to_send[config_defs::right_ankle_RoM_idx], config_to_send[config_defs::ankle_flip_angle_dir_idx], is_left ? config_to_send[config_defs::left_ankle_torque_offset_idx] : config_to_send[config_defs::right_ankle_torque_offset_idx]), elbow((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::elbow), config_to_send, is_left ? config_to_send[config_defs::left_elbow_RoM_idx] : config_to_send[config_defs::right_elbow_RoM_idx], config_to_send[config_defs::elbow_flip_angle_dir_idx], is_left ? config_to_send[config_defs::left_elbow_torque_offset_idx] : config_to_send[config_defs::right_elbow_torque_offset_idx]) {
                                                             this->is_left = is_left;

                                                             // Set to initial levels, negative and false because we want to have as >= 0 or true when actually set and operating
                                                             this->percent_gait = -1;
                                                             this->percent_stance = -1;
                                                             this->percent_swing = -1;
                                                             this->expected_step_duration = -1;
                                                             this->expected_swing_duration = -1;
                                                             this->expected_stance_duration = -1;
                                                             this->heel_fsr = -1;
                                                             this->toe_fsr = -1;
                                                             this->do_calibration_toe_fsr = false;
                                                             this->do_calibration_refinement_toe_fsr = false;
                                                             this->do_calibration_heel_fsr = false;
                                                             this->do_calibration_refinement_heel_fsr = false;
                                                             this->ground_strike = false;
                                                             this->toe_off = false;
                                                             this->toe_strike = false;
                                                             this->toe_on = false;

                                                             this->expected_duration_window_upper_coeff = 1.75;
                                                             this->expected_duration_window_lower_coeff = 0.25;
                                                             this->ankle_angle_at_ground_strike = 0.5;

                                                             this->inclination = Inclination::Level;

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

                                                         void SideData::reconfigure(uint8_t *config_to_send) {
                                                             if ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) & this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) & !this->is_left))
                                                             {
                                                                 this->is_used = true;
                                                             }
                                                             else
                                                             {
                                                                 this->is_used = false;
                                                             }

                                                             // Reconfigure the joints
                                                             hip.reconfigure(config_to_send);
                                                             knee.reconfigure(config_to_send);
                                                             ankle.reconfigure(config_to_send);
                                                             elbow.reconfigure(config_to_send);
                                                         };
                                                         g_defs::left_knee_bias_idx]
                                       : config_to_send[config_defs::right_knee_bias_idx];
    this->loadcell_sensitive_knee = is_left ? config_to_send[config_defs::left_knee_sensitive_idx] : config_to_send[config_defs::right_knee_sensitive_idx];
    this->loadcell_bias_ankle = is_left ? config_to_send[config_defs::left_ankle_bias_idx] : config_to_send[config_defs::right_ankle_bias_idx];
    this->loadcell_sensitive_ankle = is_left ? config_to_send[config_defs::left_ankle_sensitive_idx] : config_to_send[config_defs::right_ankle_sensitive_idx];

    // IMU 관련 변수 초기화
    // imu_dist_x = 0;
    // imu_dist_y = 0;
    // imu_dist_z = 0;
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