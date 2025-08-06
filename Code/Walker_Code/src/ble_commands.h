/**
 * @file ble_commands.h
 * @author Chance Cuddeback
 * @brief This file declares the BLE commands, and the functions that should be called when they are received. 
 * @date 2022-08-22
 * 
 */

#ifndef BLE_COMMANDS_H
#define BLE_COMMANDS_H

#include "Arduino.h"
#include "ExoData.h"
#include "ParseIni.h"       //For config_defs
#include "StatusDefs.h"     //For ExoDataStatus
#include "BleMessage.h"
#include "ParamsFromSD.h"

#include "UARTHandler.h"
#include "uart_commands.h"
#include "UART_msg_t.h"
#include "Logger.h"

/**
 * @brief Type to associate a command with an ammount of data
 * 
 */
typedef struct
{
    char command;
    int length; 
} ble_command_t;

/**
 * @brief Creates a variable for each command value
 * 
 */
namespace ble_names
{
    //Recieved Commands (GUI->Firmware)
    static const char start             = 'E';
    static const char stop              = 'G';
    static const char cal_trq           = 'H';
    static const char cal_fsr           = 'L';
    static const char new_trq           = 'F';
    static const char new_fsr           = 'R';
    static const char assist            = 'c';
    static const char resist            = 'S';
    static const char motors_on         = 'x'; // 
    static const char motors_off        = 'w'; // 'w'는 모터 끄기 명령을 나타내며, ExoData의 모든 모터를 비활성화하는 데 사용됩니다
    static const char mark              = 'N'; // 'N'은 마크 명령을 나타내며, ExoData의 mark 변수를 증가시키는 데 사용됩니다
    static const char update_param      = 'f';

    //Sending Commands (Firmware->GUI) 여기에 Loadcell, IMU, Motor 데이터를 포함시키고 싶어
    static const char send_real_time_data = '?';
    static const char send_batt           = '~';
    static const char send_cal_done       = 'n';
    static const char send_error_count    = 'w';
    static const char send_trq_cal        = 'H';
    static const char send_step_count     = 's';
    static const char cal_fsr_finished    = 'n';

};

/**
 * @brief Associates the command and ammount of data that it expects to be sent/received
 * 
 */
namespace ble
{
    static const ble_command_t commands[] = 
    {
        //Recieved Commands 이 배열은 모든 BLE 명령 코드와 각 명령이 주고받을 것으로 예상되는 데이터의 개수를 매핑해
        // 데이터 길이는 float 데이터의 개수일 가능성이 높습니다. (예: length = 2는 float 2개를 의미하며, float이 4바이트라면 총 8바이트의 데이터를 기다립니다. BleParser에서 * 8이 있었습니다.
        // if (_bytes_collected == _working_message.expecting * 8) // 데이터 크기가 'expecting * 8' 바이트인 것으로 보임 (float/double 크기)

         //Recieved Commands (GUI -> Firmware)
        {ble_names::start,              0}, // 'start' 명령은 0개의 데이터를 예상함
        {ble_names::stop,               0},
        {ble_names::cal_trq,            0},
        {ble_names::cal_fsr,            0},
        {ble_names::assist,             0},
        {ble_names::resist,             0},
        {ble_names::motors_on,          0},
        {ble_names::motors_off,         0},
        {ble_names::mark,               0},
        {ble_names::new_fsr,            2}, // 'new_fsr' 명령은 2개의 데이터를 예상함 (예: 왼쪽/오른쪽 FSR 임계값)
        {ble_names::new_trq,            4},  // 'new_trq' 명령은 4개의 데이터를 예상함 (예: 관절 ID, 컨트롤러 ID, 세트 번호 등)
        {ble_names::update_param,       4}, // 'update_param' 명령은 4개의 데이터를 예상함 (예: 관절 ID, 컨트롤러 ID, 파라미터 인덱스, 파라미터 값)
        
        //Sending Commands  (Firmware -> GUI)
        {ble_names::send_batt,              1},
        {ble_names::send_real_time_data,    9}, // 'send_real_time_data' 명령은 9개의 데이터를 예상함 (예: 여러 센서 값)
        {ble_names::send_error_count,       1},
        {ble_names::send_cal_done,          0},
        {ble_names::send_trq_cal,           2}, // 로드셀로 변환
        {ble_names::send_step_count,        2}, // 'send_step_count' 명령은 2개의 데이터를 예상함 (예: 왼쪽과 오른쪽 측면의 스텝 카운트)
        {ble_names::cal_fsr_finished,       0},
    };
};

/**
 * @brief Helper function(s) to be used with the command array
 * 
 */
namespace ble_command_helpers
{
    /**
     * @brief Get the ammount of data a command is expecting
     * 
     * @param command to get the length
     * @return int amount of data for a command, -1 if command not found
     */
    inline static int get_length_for_command(char command)
    {
        int length = -1; // 기본값 -1 (명령을 찾지 못했음을 의미)

        //Get the amount of characters to wait for  `ble::commands` 배열을 순회하면서 `command`와 일치하는 명령을 찾습니다.
        for(unsigned int i=0; i < sizeof(ble::commands)/sizeof(ble::commands[0]); i++)
        {
            if(command == ble::commands[i].command)
            {
                length = ble::commands[i].length; // 해당 명령의 예상 데이터 길이를 가져와 저장
                break;
            }
        }
        return length;
    }

}

/**
 * @brief Variables used by the Handlers to track state
 * 
 */
namespace ble_handler_vars
{
    //Should be used sparingly, we chose to do this so that ExoData wasn't needlessly populated with variables
    static const uint8_t k_max_joints = 6; // 최대 관절 개수 (6개로 고정)
    static uint8_t prev_controllers[k_max_joints] = {0, 0, 0, 0, 0, 0}; // 각 관절의 이전 컨트롤러 ID를 저장하는 배열

}

/**
 * @brief Holds the functions that should be called when a command is received. All command handlers should have 
 * static linkage, return void, and accept a pointer to ExoData.
 * ie "inline static void my_handler(ExoData* data, BleMessage* msg)"
 * 
 */
namespace ble_handlers
{
    inline static void start(ExoData* data, BleMessage* msg)
    {
        //Start the trial (ie Enable motors and begin streaming data). If the joint is used; enable the motor, and set the controller to zero torque

        // 1. 모든 사용 중인 관절의 모터 활성화
        data->for_each_joint(  // 모든 관절에 대해 람다 함수 실행
            
            // This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used) // 이 관절이 사용 중이라면
                {
                    j_data->motor.enabled = 1; // 해당 관절의 모터를 활성화
                }
                return;
            }
        );

        //Set the data status to running 
        data->set_status(status_defs::messages::trial_on);

        //Send status update  // UART를 통해 다른 MCU에 상태 업데이트 명령 전송
        UARTHandler* uart_handler = UARTHandler::get_instance(); // UART 핸들러 인스턴스 가져오기
        UART_msg_t tx_msg; // 보낼 UART 메시지 생성
        tx_msg.command = UART_command_names::update_status; // 명령: 상태 업데이트
        tx_msg.joint_id = 0; // 모든 관절에 해당하므로 0 (전역)
        tx_msg.data[(uint8_t)UART_command_enums::status::STATUS] = data->get_status();
        tx_msg.len = (uint8_t)UART_command_enums::status::LENGTH;
        uart_handler->UART_msg(tx_msg);

        delayMicroseconds(10);

        //Send motor enable update  UART를 통해 모터 활성화 상태 업데이트 명령 전송
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 1;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);

        delayMicroseconds(10); // 짧은 지연

        //Send FSR Calibration and Refinement  UART를 통해 FSR 보정/미세 조정 업데이트 명령 전송
        tx_msg.command = UART_command_names::update_cal_fsr;
        tx_msg.len = 0;
        uart_handler->UART_msg(tx_msg);
    }
    inline static void stop(ExoData* data, BleMessage* msg) // 모든 사용 중인 관절의 모터 비활성화 (start의 역과정)
    {
        //Stop the trial (inverse of start) & send trial summary data (step information)
        data->for_each_joint(
            
            //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 0;
                }
                return;
            }
        );

        //Set the data status to off 시스템 상태를 '정지'로 설정
        data->set_status(status_defs::messages::trial_off);

        //Send status update
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_status;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::status::STATUS] = data->get_status();
        tx_msg.len = (uint8_t)UART_command_enums::status::LENGTH;
        uart_handler->UART_msg(tx_msg);

        delayMicroseconds(100); // 지연

        //Send motor enable update UART를 통해 다른 MCU에 상태 업데이트 명령 전송
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 0;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);

        data->mark = 10;  // `data->mark` 변수를 10으로 설정 (특정 이벤트 마킹용)
}
    }
    inline static void cal_trq(ExoData* data, BleMessage* msg) // 토크센서 보정
    {   
        //Raise cal_trq flag for all joints being used, (Out of context: Should send calibration info upon cal completion)
        data->for_each_joint([](JointData* j_data, float* args) {j_data->calibrate_torque_sensor = j_data->is_used;});

        //Send cal_trq 토크 센서 보정 업데이트 명령 전송
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_cal_trq_sensor;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::cal_trq_sensor::CAL_TRQ_SENSOR] = 1;
        tx_msg.len = (uint8_t)UART_command_enums::cal_trq_sensor::LENGTH;
        uart_handler->UART_msg(tx_msg);
    }
    inline static void cal_fsr(ExoData* data, BleMessage* msg)
    {
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_cal_fsr;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::cal_fsr::CAL_FSR] = 1;
        tx_msg.len = (uint8_t)UART_command_enums::cal_fsr::LENGTH;
        uart_handler->UART_msg(tx_msg);
        tx_msg.command = UART_command_names::update_refine_fsr;
        tx_msg.len = 0;
        uart_handler->UART_msg(tx_msg);
    }


    // 향후 업데이트 가능
    inline static void assist(ExoData* data, BleMessage* msg)
    {
        //Right now we are approaching defining assistance and resistance directly in the controllers via a controller specific parameter, future work may populate these functions instead
    }
    inline static void resist(ExoData* data, BleMessage* msg)
    {
        //Right now we are approaching defining assistance and resistance directly in the controllers via a controller specific parameter, future work may populate these functions instead
    }


    // GUI로부터 '모터 켜기' 명령을 받으면, 모든 사용 중인 모터를 활성화하고 이 상태를 내부 MCU에 UART로 알립니다
    inline static void motors_on(ExoData* data, BleMessage* msg)
    {        
        data->for_each_joint(
            
            //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 1;
                }
                return;
            }
        );

        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 1;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);
        
    }


    // GUI로부터 '모터 끄기' 명령을 받으면, 모든 사용 중인 모터를 비활성화하고 이 상태를 내부 MCU에 UART로 알립니다
    inline static void motors_off(ExoData* data, BleMessage* msg)
    {   
        data->for_each_joint(
            
            //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 0;
                }
                return;
            }
        );

        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 0;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);

    }


    // `data->mark` 변수를 1 증가시킵니다.
    // "Real time signals에서 다른 데이터를 보냄으로써 이루어질 수 있음. flag를 올리거나 exo_data의 변수를 증가시켜야 함"
    // GUI로부터 '마크' 명령을 받으면, ExoData에 있는 mark 변수(주로 타이밍 또는 이벤트 추적용)를 1 증가시킵니다. 이는 데이터 로깅 시 특정 시점을 표시하는 데 사용
    inline static void mark(ExoData* data, BleMessage* msg)
    {
        //Increment mark variable (Done by sending different data on one of the real time signals, we should raise a flag or inc a var in exo_data)
        data->mark++;
    }



    //   GUI로부터 '새로운 토크 컨트롤러/파라미터 세트' 명령 받으면 해당 관절의 ControllerData를 업데이트
    inline static void new_trq(ExoData* data, BleMessage* msg)
    {
        //(LSP, LDSP, RSP, RDSP) Unpack message data
        config_defs::joint_id joint_id = (config_defs::joint_id)msg->data[0];
        uint8_t controller_id = (uint8_t)msg->data[1];
        uint8_t set_num = (uint8_t)msg->data[2];
        
        //Update Exo_Data controller for each joint
        ControllerData* cont_data = NULL;

        //Map the joint IDs because the GUI limits the maximum number for the message
        joint_id = (joint_id==(config_defs::joint_id)1)?(data->left_side.hip.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)2)?(data->left_side.knee.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)3)?(data->left_side.ankle.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)4)?(data->left_side.elbow.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)5)?(data->right_side.hip.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)6)?(data->right_side.knee.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)7)?(data->right_side.ankle.id):(joint_id);
        joint_id = (joint_id==(config_defs::joint_id)8)?(data->right_side.elbow.id):(joint_id);

        if (joint_id == data->left_side.ankle.id)
        {
            cont_data = &data->left_side.ankle.controller;
        } 
        else if (joint_id == data->left_side.knee.id) 
        {
            cont_data = &data->left_side.knee.controller;
        } 
        else if (joint_id == data->left_side.hip.id)
        {
            cont_data = &data->left_side.hip.controller;
        } 
        else if (joint_id == data->left_side.elbow.id)
        {
            cont_data = &data->left_side.elbow.controller;
        }
        else if (joint_id == data->right_side.ankle.id) 
        {
            cont_data = &data->right_side.ankle.controller;
        } 
        else if (joint_id == data->right_side.knee.id) 
        {
            cont_data = &data->right_side.knee.controller;
        } 
        else if (joint_id == data->right_side.hip.id) 
        {
            cont_data = &data->right_side.hip.controller;
        }
        else if (joint_id == data->right_side.elbow.id)
        {
            cont_data = &data->right_side.elbow.controller;
        }
        if (cont_data == NULL) {
            logger::println("cont_data is NULL!", LogLevel::Warn);
        }
        if (cont_data != NULL) {
            cont_data->controller = controller_id;
            cont_data->parameter_set = set_num;
        }

        //Set_controller_params((uint8_t)joint_id, controller_id, set_num, data);
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_controller_params;
        tx_msg.joint_id = (uint8_t) joint_id;
        tx_msg.data[(uint8_t)UART_command_enums::controller_params::CONTROLLER_ID] = controller_id;
        tx_msg.data[(uint8_t)UART_command_enums::controller_params::PARAM_LENGTH] = 1;
        tx_msg.data[(uint8_t)UART_command_enums::controller_params::PARAM_START] = set_num;
        tx_msg.len = 3;
        uart_handler->UART_msg(tx_msg);

        UART_msg_t_utils::print_msg(tx_msg);
    }



    // GUI로부터 '새로운 FSR 임계값' 명령을 받으면, FSR 임계값을 업데이트하고 이 값을 UART를 통해 다른 MCU에 전송
    inline static void new_fsr(ExoData* data, BleMessage* msg)
    {
        //Change contact thresholds & Send UART message to update FSR thresholds
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_FSR_thesholds;
        tx_msg.joint_id = 0;
        tx_msg.len = (uint8_t)UART_command_enums::FSR_thresholds::LENGTH;
        tx_msg.data[(uint8_t)UART_command_enums::FSR_thresholds::LEFT_THRESHOLD] = msg->data[0];
        tx_msg.data[(uint8_t)UART_command_enums::FSR_thresholds::RIGHT_THRESHOLD] = msg->data[1];
        uart_handler->UART_msg(tx_msg);
    }



    // GUI로부터 '파라미터 업데이트' 명령을 받으면, 해당 파라미터를 업데이트하고 이 값을 UART를 통해 다른 MCU에 전송
    inline static void update_param(ExoData* data, BleMessage* msg)
    {
        //Send UART message to update parameter
        logger::println("ble_handlers::update_param() - Got update param message");
        logger::print("ble_handlers::update_param() - Joint ID: "); logger::println((uint8_t)msg->data[0]);
        logger::print("ble_handlers::update_param() - Controller ID: "); logger::println((uint8_t)msg->data[1]);
        logger::print("ble_handlers::update_param() - Param Index: "); logger::println((uint8_t)msg->data[2]);
        logger::print("ble_handlers::update_param() - Param Value: "); logger::println((uint8_t)msg->data[3]);
        logger::print("New message\n");

        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_controller_param;
        tx_msg.joint_id = (uint8_t) msg->data[0];
        tx_msg.data[(uint8_t)UART_command_enums::controller_param::CONTROLLER_ID] = (uint8_t) msg->data[1];
        tx_msg.data[(uint8_t)UART_command_enums::controller_param::PARAM_INDEX] = (uint8_t) msg->data[2];
        tx_msg.data[(uint8_t)UART_command_enums::controller_param::PARAM_VALUE] = (uint8_t) msg->data[3];
        tx_msg.len = 3;
        uart_handler->UART_msg(tx_msg);
    }

}

#endif