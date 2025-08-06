#include "AnkleAngles.h"
#include "Utilities.h"
#include "Logger.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// AnkleAngles* AnkleAngles::_instance = nullptr;

// AnkleAngles* AnkleAngles::GetInstance()
// {
//     if (_instance == nullptr) {
//         _instance = new AnkleAngles();
//         _instance->init();
//     }
//     return _instance;
// }


bool AnkleAngles::init(bool is_left)
{
    _left = is_left; // _left: 왼쪽 발목 센서인지 여부를 나타내는 변수
    pinMode(_left_pin, INPUT);
    pinMode(_right_pin, INPUT);     //전압 값을 읽을 준비
    _is_initialized = true;
}

float AnkleAngles::get(bool return_normalized) // 실제 발목 각도 데이터를 요청할 때
{
    if (!_is_initialized) {
        return 0; //init 함수가 먼저 호출되었는지 확인하여, 센서가 준비되지 않았다면 0을 반환하고 종료
    }
    
    //Convert ADC counts to ratio
    const int adc_counts = _left ? analogRead(_left_pin):analogRead(_right_pin);  // _left가 참이면 왼쪽 핀에서 값을 읽고, 그렇지 않으면 오른쪽 핀에서 값을 읽습니다.
    const float ratio = adc_counts / 4095.0f; // 아날로그 전압 값을 숫자(0~4095), 읽어온 숫자 값을 전체 범위(4095)로 나누어 0.0 ~ 1.0 사이의 비율 값으로 변환
    if (!return_normalized) { //return_normalized가 false이면, 이 기본 비율 값을 그대로 반환
        return ratio;
    }

    const float calibrated_ratio = _update_population_statistics(ratio);
    return calibrated_ratio;
}

float AnkleAngles::_update_population_statistics(const float new_value)
{
    _mean = utils::ewma(new_value, _mean, _ema_alpha);

    //Slowly bring the max min back to the mean
    _max_average -= _max_min_delta;
    _min_average += _max_min_delta;

    //Check for new peaks
    if (new_value > _max_average) {
        _max_average = new_value;
    } else if (new_value < _min_average) {
        _min_average = new_value;
    }

    const float range = _max_average - _min_average;

    static float return_value = 0;
    if (utils::is_outside_range(range, -0.01, 0.01)) {
        return_value = new_value / range;
    }
    
    return min(return_value, 1.4);
}


#endif