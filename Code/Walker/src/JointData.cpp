#include "JointData.h"
#include "Logger.h"

JointData::JointData(config_defs::joint_id id, uint8_t *config_to_send, float loadcell_bias, float loadcell_sensitive)
    : motor(id, config_to_send), controller(id, config_to_send)
{
    this->id = id;
    this->loadcell_bias = loadcell_bias;
    this->loadcell_sensitive = loadcell_sensitive;

    this->is_left = ((uint8_t)this->id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    this->loadcell_reading = 0.0f;
    this->position = 0.0f;
    this->velocity = 0.0f;
    this->joint_position = 0.0f;
    this->joint_velocity = 0.0f;

    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))
    {
    case (uint8_t)config_defs::joint_id::knee:
    {
        // Loadcell 사용 여부와 exo_side 설정에 따라 is_used 플래그 설정
        is_used = (config_to_send[config_defs::knee_use_Loadcell_idx] == (uint8_t)config_defs::use_Loadcell::yes) &&
                  ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        // Loadcell 사용 여부와 exo_side 설정에 따라 is_used 플래그 설정
        is_used = (config_to_send[config_defs::ankle_use_Loadcell_idx] == (uint8_t)config_defs::use_Loadcell::yes) &&
                  ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
        break;
    }
    default:
    {
        is_used = false;
        break;
    }
    }
};

void JointData::reconfigure(uint8_t *config_to_send)
{
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))
    {
    case (uint8_t)config_defs::joint_id::knee:
    {
        is_used = (config_to_send[config_defs::knee_use_Loadcell_idx] == (uint8_t)config_defs::use_Loadcell::yes) &&
                  ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        is_used = (config_to_send[config_defs::ankle_use_Loadcell_idx] == (uint8_t)config_defs::use_Loadcell::yes) &&
                  ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left) || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
        break;
    }
    default:
    {
        is_used = false;
        break;
    }
    }

    motor.reconfigure(config_to_send);
    controller.reconfigure(config_to_send);
};
