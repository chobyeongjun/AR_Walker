#include "Side.h"
#include "Logger.h"
#include <algorithm> // for std::max_element, std::min_element




#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

Side::Side(bool is_left, ExoData *exo_data)
    : _knee(
    (is_left ? config_defs::joint_id::left_knee : config_defs::joint_id::right_knee),
    exo_data), 
   _ankle(
    (is_left ? config_defs::joint_id::left_ankle : config_defs::joint_id::right_ankle),
    exo_data)
{
    _data = exo_data;
    _is_left = is_left;

    _side_data = _is_left ? &(_data->left_side) : &(_data->right_side);
    
    // Initializing state flags
    _prev_heel_off_state = true; // Use one state variable for heel off detection
    _prev_heel_strike_state = true; // Use one state variable for heel strike detection

    for (int i = 0; i < _num_steps_avg; i++)
    {
        _swing_times[i] = 0;
    }

    _heel_off_timestamp = 0;
    _prev_heel_off_timestamp = 0;

    _heel_strike_timestamp = 0;
    _prev_heel_strike_timestamp = 0;

    _expected_swing_duration = 0;
    
    // IMU threshold 초기화 (실제 보행 감지를 위한 기본값 설정)
    heel_strike_gyro_threshold = 0.0f;       // degrees/s (보행 중 각속도가 이 값 이하로 떨어지면 heel strike)  
    heel_off_gyro_threshold    = 0.0f;       // degrees/s (이 값 이상으로 올라가면 heel off)  
    
    // 타이밍 이벤트 플래그 초기화
    onset_triggered = false;
    peak_triggered = false;
    release_triggered = false;

};

void Side::disable_motors()
{
    _knee._motor->is_on = false;
    _ankle._motor->is_on = false;
};

void Side::run_side()
{
    // check_calibration();
    read_data();
    update_motor_cmds();
};

// ------------------------------------------------------------------------
// read_data(): 메인 데이터 읽기 및 이벤트 감지
// ------------------------------------------------------------------------
void Side::read_data()
{
    // 1. Joint 센서 데이터 읽기
    // 각 관절 객체의 read_data()를 호출하여 로드셀 데이터를 JointData에 저장
    if (_side_data->knee.is_used) _knee.read_data();
    if (_side_data->ankle.is_used) _ankle.read_data();

    float ankle_gyro_y = _side_data->ankle.imu_gyro_y;
    float ankle_pitch = _side_data->ankle.imu_pitch;
    
    if (_is_left) {
        ankle_gyro_y = -ankle_gyro_y;
        ankle_pitch = -ankle_pitch;
    }
    
    _side_data->heel_off = _check_heel_off(ankle_gyro_y);
    _side_data->heel_strike = _check_heel_strike(ankle_gyro_y);

    if (_side_data->heel_off) {
        _prev_heel_off_timestamp = _heel_off_timestamp;
        _heel_off_timestamp = millis();
    }
    if (_side_data->heel_strike) {
        _prev_heel_strike_timestamp = _heel_strike_timestamp;
        _heel_strike_timestamp = millis();
        
        // 새 보행 주기 시작 시 타이밍 플래그 리셋
        _reset_timing_flags();
        
        if (_prev_heel_off_timestamp > 0) {
            _side_data->expected_swing_duration = _update_expected_swing_duration();
        }
    }
    
    _side_data->percent_gait = _calc_percent_gait();
    
    // 타이밍 이벤트 계산
    _calc_gait_timing_events();
    
    // 보조력 계산
    _calc_assistance_force();
};


void Side::_calc_gait_timing_events() {
    float gcp = _side_data->percent_gait;

    // GUI에서 설정한 타이밍 값들 사용 (SideData에서 가져오기)
    if (gcp >= _side_data->onset_timing_percent && !onset_triggered) {
        _side_data->onset_event = true;
        onset_triggered = true;
    }

    if (gcp >= _side_data->peak_timing_percent && !peak_triggered) {
        _side_data->peak_event = true;
        peak_triggered = true;
    }

    if (gcp >= _side_data->release_timing_percent && !release_triggered) {
        _side_data->release_event = true;
        release_triggered = true;
    }
}

void Side::_reset_timing_flags() {
    // 새 보행 주기 시작 시 이벤트 플래그들 초기화
    onset_triggered = false;
    peak_triggered = false;
    release_triggered = false;
    
    // SideData의 이벤트 플래그들도 초기화
    _side_data->onset_event = false;
    _side_data->peak_event = false;
    _side_data->release_event = false;
    
    // 보조력 관련 상태도 초기화
    _side_data->current_reference_force = 0.0f;
    _side_data->assistance_active = false;
}

void Side::_calc_assistance_force() {
    float gcp = _side_data->percent_gait;
    float current_force = 0.0f;
    bool should_assist = false;
    
    // 보조 활성화 구간 확인 (onset ~ release 구간)
    if (gcp >= _side_data->onset_timing_percent && gcp <= _side_data->release_timing_percent) {
        should_assist = true;
        
        // Half Sine 힘 프로필 계산 (peak_timing을 중심으로)
        float total_duration = _side_data->release_timing_percent - _side_data->onset_timing_percent;
        float current_position = gcp - _side_data->onset_timing_percent;
        
        // 0 ~ PI 범위의 sine 값을 계산 (반사인파)
        float sine_angle = PI * current_position / total_duration;
        current_force = _side_data->max_reference_force * sin(sine_angle);
        
        // 추가적으로 peak timing에서 최대가 되도록 조정
        // peak_timing_percent에서 sine 값이 1이 되도록 스케일링
        float peak_position = _side_data->peak_timing_percent - _side_data->onset_timing_percent;
        float peak_angle = PI * peak_position / total_duration;
        float peak_sine = sin(peak_angle);
        
        // peak에서 최대값이 되도록 정규화
        if (peak_sine > 0) {
            current_force = current_force / peak_sine;
        }
        
        // 안전 제한: 0 이상 max_reference_force 이하로 제한
        current_force = constrain(current_force, 0.0f, _side_data->max_reference_force);
    }
    
    // SideData에 계산 결과 저장
    _side_data->current_reference_force = current_force;
    _side_data->assistance_active = should_assist;
}
  

bool Side::_check_heel_strike(float ankle_gyro_y) {
    static bool was_in_swing = false;

    // 스윙 상태였다가, 각속도가 threshold 이하로 떨어지면 Heel Strike
    if (was_in_swing && abs(ankle_gyro_y) < heel_strike_gyro_threshold) {
        was_in_swing = false;
        return true;
    }
    if (abs(ankle_gyro_y) > heel_strike_gyro_threshold) {
        was_in_swing = true;
    }
    return false;
};

bool Side::_check_heel_off(float ankle_gyro_y) {
    static bool was_in_stance = true;

    if (was_in_stance && ankle_gyro_y > heel_off_gyro_threshold) {
        was_in_stance = false;
        return true;
    }
    if (ankle_gyro_y <= heel_off_gyro_threshold) {
        was_in_stance = true;
    }
    return false;
};

float Side::_update_expected_swing_duration()
{
    // 한 스윙(swing)의 시간: Heel Off ~ 다음 Heel Strike
    unsigned int swing_time = _heel_strike_timestamp - _heel_off_timestamp;
    float expected_swing_duration = _side_data->expected_swing_duration;
    
    if (_prev_heel_off_timestamp == 0) // If the prev time isn't set just return.
    {
        return expected_swing_duration;
    }

    uint8_t num_uninitialized = 0;
    
    for (int i = 0; i < _num_steps_avg; i++)
    {
        if (_swing_times[i] == 0) {
            num_uninitialized++;
        }
    }
    
    if (num_uninitialized > 0)
    {
        for (int i = (_num_steps_avg - 1); i > 0; i--)
        {
            _swing_times[i] = _swing_times[i - 1];
        }
        _swing_times[0] = swing_time;
    }
    else
    {
        unsigned int *max_val = std::max_element(_swing_times, _swing_times + _num_steps_avg);
        unsigned int *min_val = std::min_element(_swing_times, _swing_times + _num_steps_avg);
        
        if ((swing_time <= (_side_data->expected_duration_window_upper_coeff * *max_val)) && (swing_time >= (_side_data->expected_duration_window_lower_coeff * *min_val)))
        {
            int sum_swing_times = swing_time;
            for (int i = (_num_steps_avg - 1); i > 0; i--)
            {
                sum_swing_times += _swing_times[i - 1];
                _swing_times[i] = _swing_times[i - 1];
            }
            _swing_times[0] = swing_time;

            expected_swing_duration = (float)sum_swing_times / _num_steps_avg;
        }
    }
    
    _side_data->expected_swing_duration = expected_swing_duration;
    return expected_swing_duration;
};


float Side::_calc_percent_gait()
{
    int timestamp = millis();
    int percent_gait = 0; // 초기값을 0으로 설정
    
    // 유각기(Swing Phase)의 GCP를 계산
    if (_heel_off_timestamp > 0 && _side_data->expected_swing_duration > 0)
    {
        float elapsed_time = (float)timestamp - _heel_off_timestamp;
        percent_gait = 100.0f * elapsed_time / _side_data->expected_swing_duration;
        percent_gait = min(percent_gait, 100);
    }
    return (float)percent_gait;
};

void Side::clear_swing_time_estimate()
{
    for (int i = 0; i < _num_steps_avg; i++)
    {
        _swing_times[i] = 0;
    }
};

void Side::update_motor_cmds()
{
    if (_side_data->knee.is_used)
    {
        _knee.run_joint();
    }
    if (_side_data->ankle.is_used)
    {
        _ankle.run_joint();
    }
};


#endif