/**
 * @file ExoData.h
 *
 * @brief Declares a class used to store data for the Exo to access
 *
 * @author P. Stegall
 * @date Jan. 2022
 */

#ifndef ExoData_h
#define ExoData_h

#include "Arduino.h"

#include "SideData.h"
#include <stdint.h>
#include "ParseIni.h"
#include "Board.h"
#include "StatusLed.h"
#include "StatusDefs.h"
/*
 * ExoData was broken out from the Exo class to have it mirrored on a second microcontroller that handles BLE.
 * It doesn't need to be done this way if we aren't, and is pretty cumbersome.
 * Just thought you might be wondering about the approach.
 */

// Note: Status values are in StatusDefs.h

// Type used for the for each joint method, the function should take JointData as input and return void
typedef void (*for_each_joint_function_t)(JointData *, float *);

/**
 * @brief Class to store all the data related to the exo
 */
class ExoData
{
public:
        ExoData(uint8_t *config_to_send); // Constructor // config_to_send 배열은 ParseIni.cpp 에 정의되어 있음, 48개의 키를 가진 배열로, 각 키는 외골격의 설정 정보를 나타냄
        // config_to_send에 저장된 데이터 요약 : 보드 이름, 보드 버전, 배터리 종류, 외골격 구성 정보 , 외골격 이름, 사용하는 측면, 관절별 설정, 각 관절에 사용될 모터 종류
        // 각 관절의 기어비, 각 관절의 기본 제어기 종류, 각 관절의 토크 센서 사용 여부, 하드웨어 방향 설정, 각 관절의 모터, 토크, 각도 센서 값의 방향을 뒤집을지 여부
        // 캘리브레이션 값, 각 관절의 최대 가동 범위, 각 관절의 토크 센서 오프셋

        /**
         * @brief Reconfigures the the exo data if the configuration changes after constructor called.
         *
         * @param configuration array
         */
        void reconfigure(uint8_t *config_to_send);

        /**
         * @brief performs a function for each joint
         *
         * @param pointer to the function that should be done for each used joint
         */
        template <typename F> //  F는 자료형에 얽매이지 않도록 하는 template의 매개변수, func는 **외부에서 전달받은 '함수' 자체를 담는 변수(매개변수)**

        void for_each_joint(F &&func) // F&& func는 C++11부터 도입된 고급 기능으로, 전달 참조(Forwarding Reference) 또는 보편 참조(Universal Reference)라고 부릅니다.
        // F&& func는 func라는 이름의 매개변수가 거의 모든 종류의 값(변수, 임시 값, 함수 등)을 받을 수 있게 해주는 매우 유연한 문법
        {
                func(&left_side.knee, NULL);
                func(&left_side.ankle, NULL);
                func(&right_side.knee, NULL);
                func(&right_side.ankle, NULL);
        }
        template <typename F>
        void for_each_joint(F &&func, float *args)
        {
                func(&left_side.knee, args);
                func(&left_side.ankle, args);
                func(&right_side.knee, args);
                func(&right_side.ankle, args);
        }

        // Returns a list of all of the joint IDs that are currently being used
        uint8_t get_used_joints(uint8_t *used_joints);

        /**
         * @brief Get the joint pointer for a joint id.
         *
         * @param id Joint id
         * @return JointData* Pointer to JointData class for joint with id
         */
        JointData *get_joint_with(uint8_t id);

        /**
         * @brief Prints all the exo data
         */
        void print();

        /**
         * @brief Set the status object
         *
         * @param status_to_set status_defs::messages::status_t
         */
        void set_status(uint16_t status_to_set);

        /**
         * @brief Get the status object
         *
         * @return uint16_t status_defs::messages::status_t
         */
        uint16_t get_status(void);

        /**
         * @brief Set the default controller parameters for the current controller. These are the first row in the controller csv file on the SD Card
         *
         */
        void set_default_parameters();

        /**
         * @brief Set the default controller parameters for the current controller. These are the first row in the controller csv file on the SD Card
         *
         */
        void set_default_parameters(uint8_t id);

        /**
         * @brief Start the pretrial calibration process
         *
         */

        // 센서 값의 영점을 맞추거나 사용자의 움직임 범위를 측정하는 등 정확한 데이터 측정을 위한 사전 준비 작업을 수행하는 역할
        void start_pretrial_cal();

        bool sync_led_state; /**< State of the sync led */
        bool estop;          /**< State of the estop */
        float battery_value; /**< Could be Voltage or SOC, depending on the battery type*/
        SideData left_side;  /**< Data for the left side */
        SideData right_side; /**< Data for the right side */

        uint32_t mark; /**< Used for timing, currently only used by the nano */

        uint8_t *config;    /**< Pointer to the configuration array */
        uint8_t config_len; /**< Length of the configuration array */

        //
        int error_code; /**< Current error code for the system */
        int error_joint_id;
        bool user_paused; /**< If the user has paused the system */

        int knee_torque_flag = 0;  /**< Flag to determine if we want to use torque sensor for that joint */
        int ankle_torque_flag = 0; /**< Flag to determine if we want to use torque sensor for that joint */

private:
        uint16_t _status; /**< Status of the system*/
};

#endif