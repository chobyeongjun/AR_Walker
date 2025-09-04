#ifndef ExoData_h
#define ExoData_h

#include "Arduino.h"
#include "SideData.h"
#include <stdint.h>
#include "ParseIni.h"
#include "Board.h"
#include "StatusLed.h"
#include "StatusDefs.h"
#include "Loadcell.h"
#include "IMU.h"

// JointData 타입의 객체를 가리키는 포인터. float 타입의 변수를 가리키는 포인터.
typedef void (*for_each_joint_function_t)(JointData *, float *); // for_each_joint_function_t: 새롭게 정의하는 타입의 이름

class ExoData
{
public:
        ExoData(uint8_t *config_to_send);

        void reconfigure(uint8_t *config_to_send);

        template <typename F>
        void for_each_joint(F &&func) // F&& func는 C++11부터 도입된 고급 기능으로, 전달 참조(Forwarding Reference) 또는 보편 참조(Universal Reference)라고 부릅니다.
        // F&& func는 func라는 이름의 매개변수가 거의 모든 종류의 값(변수, 임시 값, 함수 등)을 받을 수 있게 해주는 매우 유연한 문법
        {
                func(&left_side.knee, NULL);
                func(&left_side.ankle, NULL);
                func(&right_side.knee, NULL);
                func(&right_side.ankle, NULL);
        }
        template <typename F>
        void for_each_joint(F &&func, float *args) // 객체의 메모리 주소, args는 함수에 전달할 추가 인자
        {

                func(&left_side.knee, args);
                func(&left_side.ankle, args);
                func(&right_side.knee, args);
                func(&right_side.ankle, args);
        }

        uint8_t get_used_joints(uint8_t *used_joints);

        JointData *get_joint_with(uint8_t id);

        void print();

        void set_status(uint16_t status_to_set);

        uint16_t get_status(void);

        void set_default_parameters();

        void set_default_parameters(uint8_t id);

        void reset_imu_data();

        bool sync_led_state; /**< State of the sync led */
        bool estop;          /**< State of the estop */
        float battery_value; /**< Could be Voltage or SOC, depending on the battery type*/
        float battery_percent; /**< LiPo battery percentage (0-100%) */

        SideData left_side;  /**< Data for the left side */
        SideData right_side; /**< Data for the right side */

        uint32_t mark = 0; /**< Used for timing, currently only used by the nano */

        uint8_t *config; /**< Pointer to the configuration array */ // uint8_t 값들의 배열이나 단일 uint8_t 변수의 위치를 기억하는
        uint8_t config_len;                                         /**< Length of the configuration array */

        int error_code; /**< Current error code for the system */
        int error_joint_id;
        bool user_paused = false; /**< If the user has paused the system */

        uint8_t knee_Loadcell_flag = 0;  /**< Flag to determine if we want to use Loadcell for that joint */
        uint8_t ankle_Loadcell_flag = 0; /**< Flag to determine if we want to use Loadcell for that joint */
        
        uint8_t knee_IMU_flag = 0;  /**< Flag to determine if we want to use IMU for that joint */
        uint8_t ankle_IMU_flag = 0; /**< Flag to determine if we want to use IMU for that joint */
        
        // Reference Force Settings for Admittance Control
        float left_knee_reference_force = 20.0f;   /**< Reference force for left knee admittance control (N) */
        float right_knee_reference_force = 18.0f;  /**< Reference force for right knee admittance control (N) */
        float left_ankle_reference_force = 25.0f;  /**< Reference force for left ankle admittance control (N) */
        float right_ankle_reference_force = 22.0f; /**< Reference force for right ankle admittance control (N) */
        
        // GUI Interface functions for Reference Force
        void set_reference_force(config_defs::joint_id joint_id, float force);
        float get_reference_force(config_defs::joint_id joint_id);

private:
        uint16_t _status; /**< Status of the system*/
};

#endif
