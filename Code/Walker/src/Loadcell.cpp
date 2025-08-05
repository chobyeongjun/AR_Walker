#include "Loadcell.h"
#include "Logger.h"

#define Loadcell_DEBUG 1 // Uncomment if you want to print debug statments to serial monitor.

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// 캘리브레이션 시간 (밀리초)
static const unsigned long CALIBRATION_DURATION_MS = 5000;
Loadcell::Loadcell(unsigned int pin, float reference_weight)
{
    this->_is_used = (pin == logic_micro_pins::not_connected_pin ? false : true);
    this->_pin = pin;
    this->_reference_weight = reference_weight;

    // 2점 캘리브레이션 변수 초기화
    _zero_offset_voltage = 0;
    _span_voltage = 0;
    _sensitivity = 0;

    _raw_reading = 0;
    _calculated_weight = 0; // 변수명 변경: calibrated_reading -> _calculated_weight

    _state = CalibrationState::NOT_STARTED; // 캘리브레이션 상태
    _start_time = 0;
    _calibration_sum = 0;
    _num_calibration_samples = 0;
    _last_do_calibrate = false;

    if (this->_is_used)
    {
        pinMode(this->_pin, INPUT);
    }

#ifdef Loadcell_DEBUG
    logger::print("Loadcell :: Constructor : pin ");
    logger::print(_pin);
    logger::println(" set");
#endif
};

bool Loadcell::calibrate(CalibrationState state_to_run)
{
#ifdef Loadcell_DEBUG
    logger::print("Loadcell::calibrate : state_to_run = ");
    logger::print((int)state_to_run);
    logger::print("\n");
#endif

    if (!_is_used)
        return false;

    _state = state_to_run;
    _start_time = millis();
    _calibration_sum = 0;
    _num_calibration_samples = 0;

    // 캘리브레이션 시간 동안 센서 값 측정
    while (millis() - _start_time < CALIBRATION_DURATION_MS)
    {
        _calibration_sum += analogRead(_pin) * loadcell_calibration::AI_CNT_TO_V;
        _num_calibration_samples++;
        delay(1); // 샘플링 간격
    }

    float average_voltage = 0;
    if (_num_calibration_samples > 0)
    {
        average_voltage = _calibration_sum / (float)_num_calibration_samples;
    }

    if (_state == CalibrationState::ZERO_CALIBRATING)
    {
#ifdef Loadcell_DEBUG
        logger::print("Loadcell::calibrate : ZERO_CALIBRATING Done, Voltage = ");
        logger::println(average_voltage);
#endif
        _zero_offset_voltage = average_voltage;
        return true;
    }
    else if (_state == CalibrationState::SPAN_CALIBRATING)
    {
#ifdef Loadcell_DEBUG
        logger::print("Loadcell::calibrate : SPAN_CALIBRATING Done, Voltage = ");
        logger::println(average_voltage);
#endif
        _span_voltage = average_voltage;

        // 두 점을 이용해 기울기(_sensitivity) 계산
        if ((_span_voltage - _zero_offset_voltage) != 0)
        {
            _sensitivity = _reference_weight / (_span_voltage - _zero_offset_voltage);
            _state = CalibrationState::CALIBRATED;
#ifdef Loadcell_DEBUG
            logger::print("Loadcell::calibrate : Calculated Sensitivity = ");
            logger::println(_sensitivity);
#endif
            return true;
        }
        else
        {
            _sensitivity = 0;
            _state = CalibrationState::NOT_STARTED;
            logger::println("Loadcell::calibrate : ERROR - Voltage change is zero!", LogLevel::Error);
            return false;
        }
    }
    return false;
};

float Loadcell::read()
{
    // 캘리브레이션이 완료된 경우에만 무게 계산
    if (_is_used && _state == CalibrationState::CALIBRATED)
    {
        _raw_reading = analogRead(_pin);
        float current_voltage = (float)_raw_reading * loadcell_calibration::AI_CNT_TO_V;
        _calculated_weight = (current_voltage - _zero_offset_voltage) * _sensitivity; // 0V일 때 나오는 offset을 빼서 영점을 맞추고, 기울기를 곱해서 1V당 달라지는 값을 무게로 출력하게 됨
        return _calculated_weight;
    }
    return 0.0f;
};

float Loadcell::read_with_offset_and_sensitivity(float zero_offset, float sensitivity)
{
    if (_is_used)
    {
        _raw_reading = analogRead(_pin);
        float current_voltage = (float)_raw_reading * loadcell_calibration::AI_CNT_TO_V;
        _calculated_weight = (current_voltage - zero_offset) * sensitivity;
        return _calculated_weight;
    }
    return 0.0f;
};

#endif
