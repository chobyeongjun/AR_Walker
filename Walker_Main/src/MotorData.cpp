#include "MotorData.h"
#include "ParseIni.h"
#include "Logger.h"

MotorData::MotorData(config_defs::joint_id id, uint8_t *config_to_send)
{
    this->id = id;
    this->is_left = ((uint8_t)this->id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    this->do_zero = true;
    this->is_on = true;

    // 특정 비트만 0으로 만드는 mask를 생성하는 과정 joint_id
    /*  left =  0b01000000,
        right = 0b00100000,
        knee  = 0b00000001,
        ankle = 0b00000010,
        */

    // left랑 right를 제외한 나머지 비트는 0으로
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))
    {
    case (uint8_t)config_defs::joint_id::knee:
    {
        motor_type = config_to_send[config_defs::knee_idx];

        break;

        switch (config_to_send[config_defs::knee_gear_idx])
        {
        case (uint8_t)config_defs::gearing::gearing_1_1:
        {
            gearing = 1;
            break;
        }

        default:
        {
            gearing = 1;
            break;
        }
        }
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        motor_type = config_to_send[config_defs::ankle_idx];

        break;

        switch (config_to_send[config_defs::ankle_gear_idx])
        {
        case (uint8_t)config_defs::gearing::gearing_1_1:
        {
            gearing = 1;
            break;
        }

        default:
        {
            gearing = 1;
            break;
        }
        }
    }
    default:
        // ✅ default 케이스 추가
        motor_type = config_to_send[config_defs::knee_idx];
        gearing = 1.0f;
        break;
    }

    // For AK-Series Motors Only
    p = 0; // Read position
    v = 0; // Read velocity
    i = 0; // Read current
    temperature = 0;
    error = 0;
    p_des = 0;
    v_des = 0;
    a_des = 0;

    kt = 0;
};

// 여기에서부터 데이터를 받아온 후에 motor_data를 업데이트하는 함수
void MotorData::reconfigure(uint8_t *config_to_send)
{
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right)) // Use the id with the side masked out.
    {

    case (uint8_t)config_defs::joint_id::knee:
    {
        motor_type = config_to_send[config_defs::knee_idx];

        switch (config_to_send[config_defs::knee_gear_idx])
        {
        case (uint8_t)config_defs::gearing::gearing_1_1:
        {
            gearing = 1;
            break;
        }

        default:
        {
            gearing = 1;
            break;
        }
        }
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        motor_type = config_to_send[config_defs::ankle_idx];

        switch (config_to_send[config_defs::ankle_gear_idx])
        {
        case (uint8_t)config_defs::gearing::gearing_1_1:
        {
            gearing = 1;
            break;
        }

        default:
        {
            gearing = 1;
            break;
        }
        }
    }
    }

    // For AK-Series Motors Only
    p = 0; // Read position
    v = 0; // Read velocity
    i = 0; // Read current
    temperature = 0;
    error = 0;
    p_des = 0;
    v_des = 0;
    a_des = 0;

    kt = 0;
};