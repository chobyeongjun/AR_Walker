#ifndef SideData_h
#define SideData_h

#include "Arduino.h"
#include "JointData.h"
#include "Loadcell.h"
#include "IMU.h"
#include "ParseIni.h"
#include "Board.h"
#include <stdint.h>

lass SideData {
       
    public:
        SideData(bool is_left, uint8_t* config_to_send);
        
        /**
         * @brief Reconfigures the side data if the configuration changes after constructor called.
         * * @param configuration array
         */
        void reconfigure(uint8_t* config_to_send);
        
        JointData knee;
        JointData ankle;
        
        // IMU 데이터
        float imu_acc_x;
        float imu_acc_y;
        float imu_acc_z;
        float imu_gyro_x;
        float imu_gyro_y;
        float imu_gyro_z;
        float imu_roll;
        float imu_pitch;
        float imu_yaw;
        
        // Loadcell 데이터
        float loadcell_reading;
        
        // 보행 주기 백분율 (GCP)
        float percent_gait;
        
        // Toe-off 감지를 위한 변수
        unsigned long toe_off_timestamp;
        float expected_gait_duration;
        
        // EWMA 필터 변수
        float ewma_alpha;
        
        // 캘리브레이션 관련 플래그
        CalibrationState knee_loadcell_cal_state;
        CalibrationState ankle_loadcell_cal_state;

        // IMU ID (무릎과 발목)
        uint8_t imu_id_knee;
        uint8_t imu_id_ankle;
        
        // 기타 변수
        bool is_left;
        bool is_used;
};

#endif

