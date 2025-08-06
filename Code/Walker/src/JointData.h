#ifndef JointData_h
#define JointData_h

#include "Arduino.h"
#include "MotorData.h"
#include "ControllerData.h"
#include "ParseIni.h"
#include "Board.h"
#include "Loadcell.h"

#include <stdint.h>
#include <queue>

class ExoData;

class JointData
{
public:
    // 생성자 수정
    JointData(config_defs::joint_id id, uint8_t *config_to_send, float loadcell_bias, float loadcell_sensitive);

    void reconfigure(uint8_t *config_to_send);

    config_defs::joint_id id;
    MotorData motor;
    ControllerData controller;

    float loadcell_bias;
    float loadcell_sensitive;
    float loadcell_reading;

    bool is_left;
    bool is_used;

    float position;
    float velocity;

    float joint_position;
    float joint_velocity;
    const float joint_position_alpha = 0.05f;
    const float joint_velocity_alpha = 0.05f;
}
