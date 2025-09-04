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

    // FSR-related variable initialization should be removed
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

    // 2. IMU 데이터 가져오기 및 부호 통일
    // _side_data를 통해 IMU 데이터를 가져와 부호를 통일
    float ankle_gyro_y = _side_data->_ankle.imu_gyro_y;
    float ankle_accel_z = _side_data->_ankle.imu_accel_z; // imu_accel_z는 JointData에 추가되어야 함
    float ankle_pitch = _side_data->_ankle.imu_pitch;
    if (_is_left) {
        ankle_gyro_y = -ankle_gyro_y;
        ankle_pitch = -ankle_pitch;
    }
    
    _side_data->heel_off = _check_heel_off(ankle_gyro_y, ankle_pitch);
    _side_data->heel_strike = _check_heel_strike(ankle_gyro_y, ankle_accel_z);

    if (_side_data->heel_off) {
        _prev_heel_off_timestamp = _heel_off_timestamp;
        _heel_off_timestamp = millis();
    }
    if (_side_data->heel_strike) {
        _prev_heel_strike_timestamp = _heel_strike_timestamp;
        _heel_strike_timestamp = millis();
        
        if (_prev_heel_off_timestamp > 0) {
            _side_data->expected_swing_duration = _update_expected_swing_duration();
        }
    }
    
    _side_data->percent_gait = _calc_percent_gait();
};

// void Side::check_calibration()
// {
//     if (_side_data->is_used)
//     {
//         if (_side_data->knee.is_used)
//         {
//             _knee.check_calibration();
//         }
//         if (_side_data->ankle.is_used)
//         {
//             _ankle.check_calibration();
//         }
//     }
// };

bool Side::_check_heel_strike(float ankle_gyro_y, float ankle_accel_z) {
    static bool was_in_swing = false;
    const float GYRO_THRESHOLD_STILL = 10.0f; // degrees/s
    const float ACCEL_THRESHOLD_IMPACT = 1.5f; // g

    // 스윙 상태였다가, 각속도가 0에 가깝고, 충격이 감지되면 Heel Strike
    if (was_in_swing && abs(ankle_gyro_y) < GYRO_THRESHOLD_STILL && abs(ankle_accel_z) > ACCEL_THRESHOLD_IMPACT) {
        was_in_swing = false;
        return true;
    }
    if (abs(ankle_gyro_y) > GYRO_THRESHOLD_STILL) {
        was_in_swing = true;
    }
    return false;
};

bool Side::_check_heel_off(float ankle_gyro_y, float ankle_pitch) {
    static bool was_in_stance = true;
    const float HEEL_OFF_ANGLE_THRESHOLD = 5.0f; // 5도 이상 들리면 Heel Off로 간주

    if (was_in_stance && ankle_pitch > HEEL_OFF_ANGLE_THRESHOLD) {
        was_in_stance = false;
        return true;
    }
    if (ankle_pitch <= HEEL_OFF_ANGLE_THRESHOLD) {
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

// ------------------------------------------------------------------------
// _calc_percent_gait(): GCP 계산
// ------------------------------------------------------------------------
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