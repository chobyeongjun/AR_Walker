/*
 *
 * P. Stegall Jan. 2022
 */

#include "ControllerData.h"

/*
 * Constructor for the controller data.
 * Takes the joint id and the array from the INI parser.
 * Stores the id, sets the controller to the default controller for the appropriate joint, and records the joint type to check we are using appropriate controllers.
 */
ControllerData::ControllerData(config_defs::joint_id id, uint8_t *config_to_send)
{

    switch ((uint8_t)id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right)) // Use the id with the side masked out.
    {
    case (uint8_t)config_defs::joint_id::hip:
    {
        controller = config_to_send[config_defs::exo_hip_default_controller_idx]; // 기본 컨트롤러의 ID
        joint = config_defs::JointType::hip;
        break;
    }
    case (uint8_t)config_defs::joint_id::knee:
    {
        controller = config_to_send[config_defs::exo_knee_default_controller_idx];
        joint = config_defs::JointType::knee;
        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        controller = config_to_send[config_defs::exo_ankle_default_controller_idx];
        joint = config_defs::JointType::ankle;
        break;
    }
    case (uint8_t)config_defs::joint_id::elbow:
    {
        controller = config_to_send[config_defs::exo_elbow_default_controller_idx];
        joint = config_defs::JointType::elbow;
        break;
    }
    }

    setpoint = 0;
    parameter_set = 0;

    for (int i = 0; i < controller_defs::max_parameters; i++)
    {
        parameters[i] = 0; // 제어기가 사용하는 세부 설정값 파라미터 모두 0으로 초기화
    }

    filtered_cmd = 0;
    filtered_torque_reading = 0;
};

void ControllerData::reconfigure(uint8_t *config_to_send) // 새로운 설정 정보(config_to_send)를 받아 컨트롤러 유형과 파라미터들을 재설정
{
    // Just reset controller 새로운 기본 컨트롤러 ID를 가져와 controller 멤버 변수를 업데이트
    switch ((uint8_t)joint) // Use the id with the side masked out.
    {
    case (uint8_t)config_defs::joint_id::hip:
    {
        controller = config_to_send[config_defs::exo_hip_default_controller_idx];
        break;
    }
    case (uint8_t)config_defs::joint_id::knee:
    {
        controller = config_to_send[config_defs::exo_knee_default_controller_idx];
        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        controller = config_to_send[config_defs::exo_ankle_default_controller_idx];
        break;
    }
    case (uint8_t)config_defs::joint_id::elbow:
    {
        controller = config_to_send[config_defs::exo_elbow_default_controller_idx];
        break;
    }
    }

    setpoint = 0;

    for (int i = 0; i < controller_defs::max_parameters; i++)
    {
        parameters[i] = 0;
    }
};

uint8_t ControllerData::get_parameter_length() //  현재 이 함수는 항상 0을 반환합니다. 이는 이 함수가 아직 구현되지 않았거나, 나중에 특정 컨트롤러의 파라미터 개수를 동적으로 결정할 때 사용될 예정임을 시사
{
    uint8_t length = 0;
    return length;
}