#include "ExoData.h"
#include "error_codes.h"
#include "Logger.h"
#include "ParamsFromSD.h"

ExoData::ExoData(uint8_t *config_to_send)
    : left_side(true, config_to_send), right_side(false, config_to_send)
{
    this->_status = status_defs::messages::trial_off;
    this->sync_led_state = false;
    this->estop = false;

    this->config = config_to_send;                 // config 포인터에 값을 할당
    this->config_len = ini_config::number_of_keys; // idx 개수

    this->mark = 10; // "Nano"라는 특정 마이크로컨트롤러 보드(또는 관련 시스템)에서만 사용되고 있음

    this->error_code = static_cast<int>(NO_ERROR);
    this->error_joint_id = 0;
    this->user_paused = false;

    // 전체 시스템에서 Loadcell과 IMU를 어디에 쓰고 있는지 확인하는 코드
    if ((config_to_send[config_defs::knee_use_Loadcell_idx] == (uint8_t)config_defs::use_Loadcell::yes))
    {
        knee_Loadcell_flag = 1;
    }

    if ((config_to_send[config_defs::ankle_use_Loadcell_idx] == (uint8_t)config_defs::use_Loadcell::yes))
    {
        ankle_Loadcell_flag = 1;
    }

    if ((config_to_send[config_defs::knee_use_IMU_idx] == (uint8_t)config_defs::use_IMU::yes))
    {
        knee_IMU_flag = 1;
    }

    if ((config_to_send[config_defs::ankle_use_IMU_idx] == (uint8_t)config_defs::use_IMU::yes))
    {
        ankle_IMU_flag = 1;
    }
};

void ExoData::reconfigure(uint8_t *config_to_send)
{
    left_side.reconfigure(config_to_send);
    right_side.reconfigure(config_to_send);
};

uint8_t ExoData::get_used_joints(uint8_t *used_joints) // 현재 사용 중인 관절의 목록을 만들고, 사용 중인 관절의 총개수를 알려주는
{
    uint8_t len = 0;

    used_joints[len] = ((left_side.knee.is_used) ? (1) : (0));
    len += left_side.knee.is_used;
    used_joints[len] = ((left_side.ankle.is_used) ? (1) : (0));
    len += left_side.ankle.is_used;
    used_joints[len] = ((right_side.knee.is_used) ? (1) : (0));
    len += right_side.knee.is_used;
    used_joints[len] = ((right_side.ankle.is_used) ? (1) : (0));
    len += right_side.ankle.is_used;

    return len;
};

// JointData 타입의 객체를 가리키는 포인터를 반환한다는 의미
JointData *ExoData::get_joint_with(uint8_t id)
{
    JointData *j_data = NULL; //
    switch (id)
    {
    case (uint8_t)config_defs::joint_id::left_knee:
        j_data = &left_side.knee;
        break;
    case (uint8_t)config_defs::joint_id::left_ankle:
        j_data = &left_side.ankle;
        break;
    case (uint8_t)config_defs::joint_id::right_knee:
        j_data = &right_side.knee;
        break;
    case (uint8_t)config_defs::joint_id::right_ankle:
        j_data = &right_side.ankle;
        break;

    default:
        // logger::print("ExoData::get_joint_with->No joint with ");
        // logger::print(id);
        // logger::println(" was found.");
        break;
    }
    return j_data;
};

void ExoData::set_status(uint16_t status_to_set)
{
    // If the status is already error, don't change it
    if (this->_status == status_defs::messages::error)
    {
        return;
    }
    this->_status = status_to_set;
}

uint16_t ExoData::get_status(void)
{
    return this->_status;
}

void ExoData::set_default_parameters()
{
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    this->for_each_joint([this](JointData *j_data, float *args)
                         {
            if (j_data->is_used)
            {
                set_controller_params((uint8_t)j_data->id, j_data->controller.controller, 0, this);
            } });
#endif
}

void ExoData::set_default_parameters(uint8_t id)
{
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    float f_id = static_cast<float>(id);
    this->for_each_joint(
        [this](JointData *j_data, float *args)
        {
            if (j_data->is_used && (uint8_t)j_data->id == static_cast<uint8_t>(args[0]))
            {
                set_controller_params((uint8_t)j_data->id, j_data->controller.controller, 0, this);
            }
        },
        &f_id);
#endif
}

void ExoData::start_pretrial_cal()
{
    // Calibrate the Torque Sensors
    this->for_each_joint([](JointData *j_data, float *args)
                         { j_data->calibrate_torque_sensor = j_data->is_used; });
}

void ExoData::print()
{
    logger::print("\t Status : ");
    logger::println(_status);
    logger::print("\t Sync LED : ");
    logger::println(sync_led_state);

    if (left_side.is_used)
    {
        logger::println("\tLeft :: Data");

        // IMU 데이터 출력
        logger::print("\t\tIMU Distance (x,y,z) : ");
        logger::print(left_side.imu_dist_x);
        logger::print(", ");
        logger::print(left_side.imu_acc_y);
        logger::print(", ");
        logger::println(left_side.imu_acc_z);
        logger::print("\t\tIMU Gyro (x,y,z) : ");
        logger::print(left_side.imu_gyro_x);
        logger::print(", ");
        logger::print(left_side.imu_gyro_y);
        logger::print(", ");
        logger::println(left_side.imu_gyro_z);
        logger::print("\t\tIMU Angle (r,p,y) : ");
        logger::print(left_side.imu_roll);
        logger::print(", ");
        logger::print(left_side.imu_pitch);
        logger::print(", ");
        logger::println(left_side.imu_yaw);

        // IMU ID를 더 명확하게 출력
        logger::print("\t\tIMU ID (Knee) : ");
        logger::println(left_side.imu_id_knee);
        logger::print("\t\tIMU ID (Ankle) : ");
        logger::println(left_side.imu_id_ankle);

        // GCP 데이터 출력
        logger::print("\t\tGCP : ");
        logger::println(left_side.percent_gait);
        logger::print("\t\tExpected Gait Duration : ");
        logger::println(left_side.expected_gait_duration);

        // 무릎 관절 데이터 출력
        if (left_side.knee.is_used)
        {
            logger::println("\tLeft :: Knee");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(left_side.knee.loadcell_reading);
            logger::print("\t\tLoadcell Bias : ");
            logger::println(left_side.knee.loadcell_bias);
            logger::print("\t\tLoadcell Sensitive : ");
            logger::println(left_side.knee.loadcell_sensitive);
        }

        // 발목 관절 데이터 출력
        if (left_side.ankle.is_used)
        {
            logger::println("\tLeft :: Ankle");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(left_side.ankle.loadcell_reading);
            logger::print("\t\tLoadcell Bias : ");
            logger::println(left_side.ankle.loadcell_bias);
            logger::print("\t\tLoadcell Sensitive : ");
            logger::println(left_side.ankle.loadcell_sensitive);
        }
    }

    if (right_side.is_used)
    {
        logger::println("\tRight :: Data");

        // IMU 데이터 출력
        logger::print("\t\tIMU Accel (x,y,z) : ");
        logger::print(right_side.imu_acc_x);
        logger::print(", ");
        logger::print(right_side.imu_acc_y);
        logger::print(", ");
        logger::println(right_side.imu_acc_z);
        logger::print("\t\tIMU Gyro (x,y,z) : ");
        logger::print(right_side.imu_gyro_x);
        logger::print(", ");
        logger::print(right_side.imu_gyro_y);
        logger::print(", ");
        logger::println(right_side.imu_gyro_z);
        logger::print("\t\tIMU Angle (r,p,y) : ");
        logger::print(right_side.imu_roll);
        logger::print(", ");
        logger::print(right_side.imu_pitch);
        logger::print(", ");
        logger::println(right_side.imu_yaw);

        // IMU ID를 더 명확하게 출력
        logger::print("\t\tIMU ID (Knee) : ");
        logger::println(right_side.imu_id_knee);
        logger::print("\t\tIMU ID (Ankle) : ");
        logger::println(right_side.imu_id_ankle);

        // GCP 데이터 출력
        logger::print("\t\tGCP : ");
        logger::println(right_side.percent_gait);
        logger::print("\t\tExpected Gait Duration : ");
        logger::println(right_side.expected_gait_duration);

        // 무릎 관절 데이터 출력
        if (right_side.knee.is_used)
        {
            logger::println("\tRight :: Knee");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(right_side.knee.loadcell_reading);
            logger::print("\t\tLoadcell Bias : ");
            logger::println(right_side.knee.loadcell_bias);
            logger::print("\t\tLoadcell Sensitive : ");
            logger::println(right_side.knee.loadcell_sensitive);
        }

        // 발목 관절 데이터 출력
        if (right_side.ankle.is_used)
        {
            logger::println("\tRight :: Ankle");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(right_side.ankle.loadcell_reading);
            logger::print("\t\tLoadcell Bias : ");
            logger::println(right_side.ankle.loadcell_bias);
            logger::print("\t\tLoadcell Sensitive : ");
            logger::println(right_side.ankle.ankle.loadcell_sensitive);
        }
    }
};