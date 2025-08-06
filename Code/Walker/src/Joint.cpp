#include "Joint.h"
#include "Time_Helper.h"
#include "Logger.h"
#include "ErrorReporter.h"
#include "error_codes.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

uint8_t _Joint::left_loadcell_used_count = 0;
uint8_t _Joint::right_loadcell_used_count = 0;

uint8_t _Joint::left_motor_used_count = 0;
uint8_t _Joint::right_motor_used_count = 0;

_Joint::_Joint(config_defs::joint_id id, ExoData *exo_data)
    : _loadcell(get_loadcell_pin(id, exo_data))
{
    _id = id;
    _is_left = utils::get_is_left(_id);
    _data = exo_data;
    _motor = nullptr;
    _controller = nullptr;
    _joint_data = nullptr;
    // 이 생성자에서는 JointData가 아직 초기화되지 않았으므로
    // _joint_data 포인터는 Side.cpp의 생성자에서 할당
};

void _Joint::read_data()
{
    if (_joint_data == nullptr)
    {
        return;
    }

    // Loadcell 값 읽기
    float loadcell_bias = _joint_data->loadcell_bias;
    float loadcell_sensitive = _joint_data->loadcell_sensitive;

    _joint_data->loadcell_reading = _loadcell.read(loadcell_bias, loadcell_sensitive);

    // 모터 위치, 속도 업데이트
    _joint_data->position = _joint_data->motor.p / _joint_data->motor.gearing;
    _joint_data->velocity = _joint_data->motor.v / _joint_data->motor.gearing;
}
