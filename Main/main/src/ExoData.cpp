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

void ExoData::reset_imu_data()
{
    if (knee_IMU_flag) {
        left_side.knee.imu_pitch = 0.0f;
        left_side.knee.imu_gyro_y = 0.0f;
        right_side.knee.imu_pitch = 0.0f;
        right_side.knee.imu_gyro_y = 0.0f;
    }
    if (ankle_IMU_flag) {
        left_side.ankle.imu_pitch = 0.0f;
        left_side.ankle.imu_gyro_y = 0.0f;
        right_side.ankle.imu_pitch = 0.0f;
        right_side.ankle.imu_gyro_y = 0.0f;
    }
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

        // 무릎 IMU 데이터 출력
        if (left_side.knee.is_used) {
            logger::println("\tLeft :: Knee IMU");
            logger::print("\t\tIMU Pitch : ");
            logger::println(left_side.knee.imu_pitch);
            logger::print("\t\tIMU Gyro Y : ");
            logger::println(left_side.knee.imu_gyro_y);
        }

        // 발목 IMU 데이터 출력
        if (left_side.ankle.is_used) {
            logger::println("\tLeft :: Ankle IMU");
            logger::print("\t\tIMU Pitch : ");
            logger::println(left_side.ankle.imu_pitch);
            logger::print("\t\tIMU Gyro Y : ");
            logger::println(left_side.ankle.imu_gyro_y);
        }

        // GCP 데이터 출력
        logger::print("\t\tGCP : ");
        logger::println(left_side.percent_gait);
   

        // 무릎 관절 Loadcell 데이터 출력
        if (left_side.knee.is_used)
        {
            logger::println("\tLeft :: Knee");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(left_side.knee.loadcell_reading);
            
        }

        // 발목 관절 Loadcell 데이터 출력
        if (left_side.ankle.is_used)
        {
            logger::println("\tLeft :: Ankle");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(left_side.ankle.loadcell_reading);
            
        }
    }

    if (right_side.is_used)
    {
        logger::println("\tRight :: Data");

        // 무릎 IMU 데이터 출력
        if (right_side.knee.is_used) {
            logger::println("\tRight :: Knee IMU");
            logger::print("\t\tIMU Pitch : ");
            logger::println(right_side.knee.imu_pitch);
            logger::print("\t\tIMU Gyro Y : ");
            logger::println(right_side.knee.imu_gyro_y);
        }

        // 발목 IMU 데이터 출력
        if (right_side.ankle.is_used) {
            logger::println("\tRight :: Ankle IMU");
            logger::print("\t\tIMU Pitch : ");
            logger::println(right_side.ankle.imu_pitch);
            logger::print("\t\tIMU Gyro Y : ");
            logger::println(right_side.ankle.imu_gyro_y);
        }

        // GCP 데이터 출력
        logger::print("\t\tGCP : ");
        logger::println(right_side.percent_gait);


        // 무릎 관절 Loadcell 데이터 출력
        if (right_side.knee.is_used)
        {
            logger::println("\tRight :: Knee");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(right_side.knee.loadcell_reading);
        }

        // 발목 관절 Loadcell 데이터 출력
        if (right_side.ankle.is_used)
        {
            logger::println("\tRight :: Ankle");
            logger::print("\t\tLoadcell Reading : ");
            logger::println(right_side.ankle.loadcell_reading);
        }
    }
};

void ExoData::set_reference_force(config_defs::joint_id joint_id, float force)
{
    // Apply safety limits to the reference force
    float safe_force = constrain(force, 0.0f, 50.0f);  // Limit to 0-50N for safety
    
    switch (joint_id) {
        case config_defs::joint_id::left_knee:
            left_knee_reference_force = safe_force;
            break;
        case config_defs::joint_id::right_knee:
            right_knee_reference_force = safe_force;
            break;
        case config_defs::joint_id::left_ankle:
            left_ankle_reference_force = safe_force;
            break;
        case config_defs::joint_id::right_ankle:
            right_ankle_reference_force = safe_force;
            break;
        default:
            // Invalid joint ID, do nothing
            logger::print("ExoData::set_reference_force - Invalid joint ID: ");
            logger::println((int)joint_id);
            return;
    }
    
    logger::print("Reference Force Set - Joint ID: ");
    logger::print((int)joint_id);
    logger::print(", Force: ");
    logger::print(safe_force);
    logger::println("N");
}

float ExoData::get_reference_force(config_defs::joint_id joint_id)
{
    switch (joint_id) {
        case config_defs::joint_id::left_knee:
            return left_knee_reference_force;
        case config_defs::joint_id::right_knee:
            return right_knee_reference_force;
        case config_defs::joint_id::left_ankle:
            return left_ankle_reference_force;
        case config_defs::joint_id::right_ankle:
            return right_ankle_reference_force;
        default:
            // Invalid joint ID, return default value
            logger::print("ExoData::get_reference_force - Invalid joint ID: ");
            logger::println((int)joint_id);
            return 10.0f;  // Default 10N
    }
}
