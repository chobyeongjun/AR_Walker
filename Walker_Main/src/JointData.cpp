#include "JointData.h"
#include "Logger.h"

// 생성자 시그니처 수정 및 초기화 리스트 추가
JointData::JointData(config_defs::joint_id id, uint8_t *config_to_send)
    : motor(id, config_to_send), controller(id, config_to_send)
{
    this->id = id;
    this->is_left = ((uint8_t)this->id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    this->position = 0;
    this->velocity = 0;

    this->loadcell_reading = 0;
    this->imu_pitch = 0;
    this->imu_gyro_y = 0;
    this->imu_battery = 0;
    this->parent_exo = nullptr; // 초기화 시점에는 nullptr, 나중에 설정됨

    // is_used 플래그 설정
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))
    {
    case (uint8_t)config_defs::joint_id::knee:
    {
        // Check if joint and side is used
        is_used = (config_to_send[config_defs::knee_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        // Check if joint and side is used
        is_used = (config_to_send[config_defs::ankle_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

        break;
    }
    }
};

// reconfigure 메서드 시그니처 수정 및 로직 추가
void JointData::reconfigure(uint8_t *config_to_send)
{

    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))
    {
    case (uint8_t)config_defs::joint_id::knee:
    {
        is_used = (config_to_send[config_defs::knee_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        is_used = (config_to_send[config_defs::ankle_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
        break;
    }
    }

};
