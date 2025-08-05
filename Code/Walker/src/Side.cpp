#include "Side.h"
#include "Logger.h"
#include "Utilities.h"

Side::Side(bool is_left, ExoData* exo_data)
: _hip((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::hip), exo_data)
, _knee((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::knee), exo_data)
, _ankle((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::ankle), exo_data)
, _elbow((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::elbow), exo_data)
, _imu()
{
    _is_left = is_left;
    _exo_data = exo_data;
    _side_data = is_left ? &(_exo_data->left_side) : &(_exo_data->right_side);

    if (_side_data->is_used) {
        _imu.begin(115200);
    }
}

void Side::run_side()
{
    if (_side_data->is_used) {
        check_calibration();
        read_data();
        update_motor_cmds();
    }
}

void Side::read_data()
{
    if (_side_data->imu_id_knee != 16 || _side_data->imu_id_ankle != 16) {
        _imu.readData();
        
        // Yaw 값을 Pitch로 사용
        _side_data->imu_pitch = _imu.yaw;
        
        // Gyro의 z축 값을 pitch에 대한 각속도로 사용
        _side_data->imu_gyro_y = _imu.gyro_z;
        
        // 좌우 반전 로직 적용
        if (!_is_left) { // 오른쪽인 경우
            _side_data->imu_pitch = -_side_data->imu_pitch;
            _side_data->imu_gyro_y = -_side_data->imu_gyro_y;
        }
    }

    // Loadcell 데이터 읽기
    if (_side_data->knee.is_used) {
        _knee.read_data();
        if (!_is_left) { // 오른쪽인 경우
            _side_data->knee.loadcell_reading = -_side_data->knee.loadcell_reading;
        }
    }
    if (_side_data->ankle.is_used) {
        _ankle.read_data();
        if (!_is_left) { // 오른쪽인 경우
            _side_data->ankle.loadcell_reading = -_side_data->ankle.loadcell_reading;
        }
    }
    
    // GCP 계산
    if (_check_imu_toe_off()) {
        _update_gait_duration();
    }
    _side_data->percent_gait = _calc_percent_gait();
}

void Side::check_calibration()
{
    if (_side_data->knee.is_used) {
        _knee.check_calibration();
    }
    if (_side_data->ankle.is_used) {
        _ankle.check_calibration();
    }
}

void Side::update_motor_cmds()
{
    if (_side_data->knee.is_used) {
        _knee.run_joint();
        if (!_is_left) { // 오른쪽인 경우
            _side_data->knee.controller.setpoint = -_side_data->knee.controller.setpoint;
        }
    }
    if (_side_data->ankle.is_used) {
        _ankle.run_joint();
        if (!_is_left) { // 오른쪽인 경우
            _side_data->ankle.controller.setpoint = -_side_data->ankle.controller.setpoint;
        }
    }
}

void Side::disable_motors()
{
    _knee.disable_motor();
    _ankle.disable_motor();
}

bool Side::_check_imu_toe_off() {
    _prev_ankle_angle = _current_ankle_angle;
    _current_ankle_angle = _side_data->imu_pitch;
    _prev_gyro_y = _current_gyro_y;
    _current_gyro_y = _side_data->imu_gyro_y;

    const float GYRO_ZERO_THRESHOLD = 0.5f;
    bool current_is_plantarflexion = _current_gyro_y < -GYRO_ZERO_THRESHOLD;
    
    if (_was_in_plantarflexion && !current_is_plantarflexion) {
        _was_in_plantarflexion = false;
        return true;
    }

    _was_in_plantarflexion = current_is_plantarflexion;
    return false;
}

void Side::_update_gait_duration() {
    if (_side_data->toe_off_timestamp != 0) {
        unsigned long current_gait_duration = millis() - _side_data->toe_off_timestamp;
        
        if (_side_data->expected_gait_duration == 0.0f) {
            _side_data->expected_gait_duration = (float)current_gait_duration;
        } else {
            _side_data->expected_gait_duration = utils::ewma(
                (float)current_gait_duration,
                _side_data->expected_gait_duration,
                _side_data->ewma_alpha
            );
        }
    }
    _side_data->toe_off_timestamp = millis();
}

float Side::_calc_percent_gait() {
    if (_side_data->expected_gait_duration > 0.0f) {
        unsigned long current_time = millis();
        unsigned long time_since_toe_off = current_time - _side_data->toe_off_timestamp;
        
        float gcp = (float)time_since_toe_off / _side_data->expected_gait_duration * 100.0f;
        
        return constrain(gcp, 0.0f, 100.0f);
    }
    return 0.0f;
}

void Side::set_controller(int joint, int controller)
{
    if (_knee.get_id() == joint) {
        _knee.set_controller(controller);
    } else if (_ankle.get_id() == joint) {
        _ankle.set_controller(controller);
    }
}

void Side::clear_step_time_estimate()
{
    _side_data->toe_off_timestamp = 0;
    _side_data->expected_gait_duration = 0.0f;
    _side_data->percent_gait = 0.0f;
}