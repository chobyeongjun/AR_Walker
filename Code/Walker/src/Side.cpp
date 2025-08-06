#include "Side.h"
#include "Logger.h"
#include "Utilities.h"
#include <cmath>     // sqrt() 함수 사용
#include <algorithm> // constrain() 함수가 여기에 있을 수 있음 (혹은 Utilities.h)

Side::Side(bool is_left, ExoData *exo_data)
    : _knee((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::knee), exo_data),
      _ankle((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::ankle), exo_data),
      _imu()
{
    _is_left = is_left;
    _exo_data = exo_data;
    _side_data = is_left ? &(_exo_data->left_side) : &(_exo_data->right_side);

    if (_side_data->is_used)
    {
        _imu.begin(115200);
    }
}

void Side::run_side()
{
    if (_side_data->is_used)
    {
        read_data();
        update_motor_cmds();
    }
}

void Side::read_data()
{
    // 1. 모든 센서로부터 최신 데이터를 읽어옵니다.
    if (_side_data->is_used && (_side_data->imu_id_knee != 16 || _side_data->imu_id_ankle != 16))
    {
        _imu.readData();

        if (_imu.get_id() == _side_data->imu_id_knee)
        {
            _side_data->imu_dist_x = _imu.dist_x;
            _side_data->imu_dist_y = _imu.dist_y;
            _side_data->imu_dist_z = _imu.dist_z;
            _side_data->imu_gyro_x = _imu.gyro_x;
            _side_data->imu_gyro_y = -_imu.gyro_z; // left gyro를 plantar 일 떄 -로 만들기 위함.
            _side_data->imu_gyro_z = _imu.gyro_y;
            _side_data->imu_roll = _imu.roll;
            _side_data->imu_pitch = _imu.yaw;
            _side_data->imu_yaw = _imu.pitch;

            if (!_is_left)
            {
                _side_data->imu_pitch = -_side_data->imu_pitch; // right yaw를 plantar 일 떄 -로 만들기 위함.
                _side_data->imu_gyro_y = _side_data->imu_gyro_y;
            }
        }
    }

    if (_side_data->knee.is_used)
    {
        _knee.read_data();
        if (!_is_left)
            _side_data->knee.loadcell_reading = -_side_data->knee.loadcell_reading;
    }
    if (_side_data->ankle.is_used)
    {
        _ankle.read_data();
        if (!_is_left)
            _side_data->ankle.loadcell_reading = -_side_data->ankle.loadcell_reading;
    }

    // 2. 읽어온 데이터를 기반으로 보행 상태와 임계값을 업데이트합니다.
    _update_gait_state();

    // 3. 업데이트된 상태를 사용하여 보행 이벤트를 감지합니다.
    if (_check_imu_heel_off())
    {
        _side_data->heel_off_timestamp = millis();
    }

    if (_check_imu_toe_off())
    {
        _update_gait_duration();
    }

    // 4. 현재 보행 주기(%)를 계산합니다.
    _side_data->percent_gait = _calc_percent_gait();
}

void Side::update_motor_cmds()
{
    if (_side_data->is_used)
    {
        if (_side_data->knee.is_used)
        {
            _knee.run_joint();
            if (!_is_left)
                _side_data->knee.controller.setpoint = -_side_data->knee.controller.setpoint;
        }
        if (_side_data->ankle.is_used)
        {
            _ankle.run_joint();
            if (!_is_left)
                _side_data->ankle.controller.setpoint = -_side_data->ankle.controller.setpoint;
        }
    }
}

void Side::disable_motors()
{
    _knee.disable_motor();
    _ankle.disable_motor();
}

void Side::_update_gait_state()
{
    // 현재 값을 이전 값으로 옮기고, 새 값을 현재 값으로 업데이트
    _prev_ankle_angle = _current_ankle_angle;
    _current_ankle_angle = _side_data->imu_pitch;
    _prev_gyro_y = _current_gyro_y;
    _current_gyro_y = _side_data->imu_gyro_y;

    // 새 자이로 데이터를 기반으로 두 가지 적응형 임계값을 모두 업데이트
    _update_adaptive_thresholds();
}

bool Side::_check_imu_heel_off()
{
    // '정지 상태'에서 '움직임 상태'로 전환되는 순간을 감지
    bool was_still = abs(_prev_gyro_y) < _adaptive_gyro_stillness_threshold;
    bool is_moving_into_plantarflexion = _current_gyro_y < -_adaptive_gyro_threshold;

    if (was_still && is_moving_into_plantarflexion)
    {
        return true;
    }
    return false;
}

bool Side::_check_imu_toe_off()
{
    bool current_is_plantarflexion = _current_gyro_y < -_adaptive_gyro_threshold;

    if (_was_in_plantarflexion && !current_is_plantarflexion)
    {
        _was_in_plantarflexion = false;
        return true;
    }

    _was_in_plantarflexion = current_is_plantarflexion;
    return false;
}

void Side::_update_gait_duration()
{
    if (_side_data->toe_off_timestamp != 0)
    {
        unsigned long current_gait_duration = millis() - _side_data->toe_off_timestamp;

        if (_side_data->expected_gait_duration == 0.0f)
        {
            _side_data->expected_gait_duration = (float)current_gait_duration;
        }
        else
        {
            _side_data->expected_gait_duration = utils::ewma(
                (float)current_gait_duration,
                _side_data->expected_gait_duration,
                _side_data->ewma_alpha);
        }
    }
    _side_data->toe_off_timestamp = millis();
}

float Side::_calc_percent_gait()
{
    if (_side_data->expected_gait_duration > 0.0f)
    {
        unsigned long time_since_toe_off = millis() - _side_data->toe_off_timestamp;
        float gcp = (float)time_since_toe_off / _side_data->expected_gait_duration * 100.0f;
        return gcp
    }
    return 0.0f;
}

void Side::_update_adaptive_thresholds()
{
    _gyro_y_buffer[_gyro_buffer_index] = _current_gyro_y;
    _gyro_buffer_index = (_gyro_buffer_index + 1) % GYRO_BUFFER_SIZE;

    if (!_gyro_buffer_filled && _gyro_buffer_index == 0)
        _gyro_buffer_filled = true;
    if (!_gyro_buffer_filled)
        return;

    // '움직임 감지' 임계값 계산 (전체 데이터 대상)
    {
        float sum = 0.0f;
        for (int i = 0; i < GYRO_BUFFER_SIZE; ++i)
            sum += _gyro_y_buffer[i];
        float mean = sum / GYRO_BUFFER_SIZE;
        float sq_diff_sum = 0.0f;
        for (int i = 0; i < GYRO_BUFFER_SIZE; ++i)
            sq_diff_sum += (_gyro_y_buffer[i] - mean) * (_gyro_y_buffer[i] - mean);
        float std_dev = sqrt(sq_diff_sum / GYRO_BUFFER_SIZE);

        _adaptive_gyro_threshold = constrain(std_dev * 3.0f, 0.2f, 1.5f);
    }

    // '정지 감지' 임계값 계산 (필터링된 데이터 대상)
    {
        float still_samples[GYRO_BUFFER_SIZE];
        int still_count = 0;
        const float NOISE_PRE_FILTER_THRESHOLD = 0.5f;

        for (int i = 0; i < GYRO_BUFFER_SIZE; ++i)
        {
            if (abs(_gyro_y_buffer[i]) < NOISE_PRE_FILTER_THRESHOLD)
            {
                still_samples[still_count++] = _gyro_y_buffer[i];
            }
        }

        if (still_count > 10)
        {
            float sum = 0.0f;
            for (int i = 0; i < still_count; ++i)
                sum += still_samples[i];
            float mean = sum / still_count;
            float sq_diff_sum = 0.0f;
            for (int i = 0; i < still_count; ++i)
                sq_diff_sum += (still_samples[i] - mean) * (still_samples[i] - mean);
            float noise_std_dev = sqrt(sq_diff_sum / still_count);

            const float MINIMUM_OFFSET = 0.05f;
            _adaptive_gyro_stillness_threshold = (noise_std_dev * 3.0f) + MINIMUM_OFFSET;
            _adaptive_gyro_stillness_threshold = constrain(_adaptive_gyro_stillness_threshold, 0.1f, 0.5f);
        }
    }
}

void Side::set_controller(int joint, int controller)
{
    if (_knee.get_id() == joint)
        _knee.set_controller(controller);
    else if (_ankle.get_id() == joint)
        _ankle.set_controller(controller);
}

void Side::clear_step_time_estimate()
{
    _side_data->toe_off_timestamp = 0;
    _side_data->heel_off_timestamp = 0; // heel_off도 함께 초기화
    _side_data->expected_gait_duration = 0.0f;
    _side_data->percent_gait = 0.0f;
    _gyro_buffer_filled = false; // 버퍼도 초기화하여 재시작 시 안정적으로 계산하도록 함
}