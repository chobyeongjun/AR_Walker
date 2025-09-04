#include "Loadcell.h"
#include "Logger.h"

// 생성자에서 id를 받아서 멤버 변수에 저장
Loadcell::Loadcell(config_defs::joint_id id, unsigned int pin)
{
    this->_id = id;
    this->_is_left = ((uint8_t)this->_id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    this->_is_used = (pin == logic_micro_pins::not_connected_pin ? false : true);
    this->_pin = pin;

    _raw_reading = 0;
    _calibrated_weight = 0;

    if (this->_is_used)
    {
        pinMode(this->_pin, INPUT);
    }
};

// read 함수가 내부 id를 기반으로 올바른 상수를 사용하도록 수정
float Loadcell::read()
{
    if (_is_used)
    {
        float bias = 0.0f;
        float sensitive = 1.0f;

        // id의 관절 종류(무릎/발목)와 방향(왼쪽/오른쪽)을 확인하여 올바른 보정값 선택
        uint8_t joint_type = (uint8_t)_id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right);

        switch (joint_type)
        {
            case (uint8_t)config_defs::joint_id::knee:
                if (_is_left) {
                    bias = loadcell_calibration::left_knee_bias;
                    sensitive = loadcell_calibration::left_knee_sensitive;
                } else {
                    bias = loadcell_calibration::right_knee_bias;
                    sensitive = loadcell_calibration::right_knee_sensitive;
                }
                break;

            case (uint8_t)config_defs::joint_id::ankle:
                if (_is_left) {
                    bias = loadcell_calibration::left_ankle_bias;
                    sensitive = loadcell_calibration::left_ankle_sensitive;
                } else {
                    bias = loadcell_calibration::right_ankle_bias;
                    sensitive = loadcell_calibration::right_ankle_sensitive;
                }
                break;
        }

        _raw_reading = analogRead(_pin);
        float current_voltage = (float)_raw_reading * loadcell_calibration::AI_CNT_TO_V;
        _calibrated_weight = (current_voltage * sensitive + bias);
        return _calibrated_weight;
    }
    return 0.0f;
};

int Loadcell::readRaw()
{
    if (_is_used)
    {
        return analogRead(_pin);
    }
    return 0;
};
