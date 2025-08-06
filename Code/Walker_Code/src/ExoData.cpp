#include "ExoData.h"
#include "error_codes.h"
#include "Logger.h"
#include "ParamsFromSD.h"

/*
 * Constructor for the exo data.
 * Takes the array from the INI parser.
 * Stores the exo status, and the sync LED state.
 * Uses an initializer list for the side data.
 */
ExoData::ExoData(uint8_t *config_to_send)
    : left_side(true, config_to_send) // Using initializer list for member objects.
      ,
      right_side(false, config_to_send) // SideData 클래스의 생성자는 첫 번째 인자로 자신이 '왼쪽'인지를 나타내는 bool 값을 받도록 설계되어 있을 가능성이 매우 높습니다.
{
    this->_status = status_defs::messages::trial_off;
    this->sync_led_state = false;
    this->estop = false;

    this->config = config_to_send;
    this->config_len = ini_config::number_of_keys;

    this->mark = 10; // "Nano"라는 특정 마이크로컨트롤러 보드(또는 관련 시스템)에서만 사용되고 있음

    this->error_code = static_cast<int>(NO_ERROR);
    this->error_joint_id = 0;
    this->user_paused = false;

    // If statement that determines if torque sensor is used for that joint (See Board.h for available torque sensor pins)
    //  이 코드에서 토크 센서를 사용하도록 설정하더라도, 안보인다면 실제 센서가 보드의 어느 핀에 물리적으로 연결되어 있는지는 Board.h 파일을 봐야 알 수 있다
    if ((config_to_send[config_defs::hip_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        hip_torque_flag = 1;
    }

    if ((config_to_send[config_defs::knee_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        knee_torque_flag = 1;
    }

    if ((config_to_send[config_defs::ankle_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        ankle_torque_flag = 1;
    }

    if ((config_to_send[config_defs::elbow_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        elbow_torque_flag = 1;
    }
};

void ExoData::reconfigure(uint8_t *config_to_send)
{
    left_side.reconfigure(config_to_send); // SideData 클래스의 reconfigure 메서드를 호출하여 왼쪽 관절의 설정을 업데이트
    right_side.reconfigure(config_to_send);
};
// SideData 클래스에서의 값을 사용함
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

JointData *ExoData::get_joint_with(uint8_t id)
{
    JointData *j_data = NULL;
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

// private 로 설정된 status 변수에 접근하는 함수
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
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41) // for_each_joint 함수는 C++의 템플릿 기능을 활용하여, 전달받다양한 자료형의 함수를 받아 사용할 수 있도록 설계되어 있습니다.
    // 그래서 for_each_joint 함수는 JointData* j_data와 float* args를 매개변수로 받는 함수를 인자로 받아서, 각 관절에 대해 해당 함수를 실행합니다.
    // func(&left_side.hip, args); 라는 곳에서 // JointData* j_data는 현재 반복 중인 관절의 데이터에 대한 주소를 나타냅니다. 우리는 주소를 넣어야 하니까.
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
    float f_id = static_cast<float>(id); // id를 float로 변환하도록 형 변환
    this->for_each_joint(                // 이름이 없는 **'일회용 즉석 함수  // 첫 번째 인자인 이 람다 함수가 나중에 호출될 때 사용할 자신만의 매개변수 목록
        [this](JointData *j_data, float *args)
        {
            if (j_data->is_used && (uint8_t)j_data->id == static_cast<uint8_t>(args[0]))
            {                                                                                       // 특정 관절의 제어기(controller) 파라미터를 0번 세트(기본값)로 설정
                set_controller_params((uint8_t)j_data->id, j_data->controller.controller, 0, this); // set_controller_params 함수는 각 관절에 대한 제어기 매개변수를 설정하는 함수입니다.
            }
        },
        &f_id // &f_id가 float* args에 해당하는 두 번째 인자
    );
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
        logger::print("\tLeft :: FSR Calibration : "); // FSR Calibration : FSR 센서의 캘리브레이션 상태를 나타냄
        logger::print(left_side.do_calibration_heel_fsr);
        logger::println(left_side.do_calibration_toe_fsr);
        logger::print("\tLeft :: FSR Refinement : "); // FSR Refinement : FSR 센서의 정제 상태를 나타냄
        logger::print(left_side.do_calibration_refinement_heel_fsr);
        logger::println(left_side.do_calibration_refinement_toe_fsr);
        logger::print("\tLeft :: Percent Gait : ");
        logger::println(left_side.percent_gait); // Percent Gait : 보행의 진행 상태를 나타내는 백분율
        logger::print("\tLeft :: Heel FSR : ");
        logger::println(left_side.heel_fsr); // heel_fsr: 왼쪽 발뒤꿈치의 압력 센서 값
        logger::print("\tLeft :: Toe FSR : ");
        logger::println(left_side.toe_fsr); // toe_fsr: 왼쪽 발가락의 압력 센서 값

        if (left_side.knee.is_used)
        {
            logger::println("\tLeft :: Knee");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(left_side.knee.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(left_side.knee.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(left_side.knee.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(left_side.knee.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(left_side.knee.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(left_side.knee.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(left_side.knee.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(left_side.knee.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(left_side.knee.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(left_side.knee.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(left_side.knee.controller.parameter_set);
        }
        if (left_side.ankle.is_used)
        {
            logger::println("\tLeft :: Ankle");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(left_side.ankle.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(left_side.ankle.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(left_side.ankle.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(left_side.ankle.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(left_side.ankle.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(left_side.ankle.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(left_side.ankle.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(left_side.ankle.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(left_side.ankle.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(left_side.ankle.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(left_side.ankle.controller.parameter_set);
        }
    }

    if (right_side.is_used)
    {
        logger::print("\tRight :: FSR Calibration : ");
        logger::print(right_side.do_calibration_heel_fsr);
        logger::println(right_side.do_calibration_toe_fsr);
        logger::print("\tRight :: FSR Refinement : ");
        logger::print(right_side.do_calibration_refinement_heel_fsr);
        logger::println(right_side.do_calibration_refinement_toe_fsr);
        logger::print("\tRight :: Percent Gait : ");
        logger::println(right_side.percent_gait);
        logger::print("\tLeft :: Heel FSR : ");
        logger::println(right_side.heel_fsr);
        logger::print("\tLeft :: Toe FSR : ");
        logger::println(right_side.toe_fsr);

        if (right_side.knee.is_used)
        {
            logger::println("\tRight :: Knee");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(right_side.knee.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(right_side.knee.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(right_side.knee.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(right_side.knee.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(right_side.knee.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(right_side.knee.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(right_side.knee.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(right_side.knee.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(right_side.knee.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(right_side.knee.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(right_side.knee.controller.parameter_set);
        }

        if (right_side.ankle.is_used)
        {
            logger::println("\tRight :: Ankle");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(right_side.ankle.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(right_side.ankle.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(right_side.ankle.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(right_side.ankle.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(right_side.ankle.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(right_side.ankle.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(right_side.ankle.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(right_side.ankle.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(right_side.ankle.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(right_side.ankle.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(right_side.ankle.controller.parameter_set);
        }
    }
};
