#ifndef JointData_h
#define JointData_h

#include "Arduino.h"

#include "MotorData.h"
#include "ControllerData.h"
#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>
#include <queue>

// Forward declaration
class ExoData;

class JointData
{
public:
    JointData(config_defs::joint_id id, uint8_t* config_to_send);

    void reconfigure(uint8_t *config_to_send);

    config_defs::joint_id id;    
    MotorData motor;             
    ControllerData controller;    

    bool is_left;                 
    bool is_used;                 

    float position;               
    float velocity;              

    float loadcell_reading;
    float imu_pitch  ;
    float imu_gyro_y ;
    uint8_t imu_battery; // IMU 배터리 수준 (0-100%)
    bool imu_time_error_detected; // IMU 시간 연속성 오류 플래그 (IMU.cpp에서 복사해옴)
    
    // Emergency/Error 브레이크 요청
    bool brake_requested = false;     // 브레이크 요청 플래그
    float brake_current = 0.0f;       // 요청된 브레이크 전류 (A)
    
    // Stance phase continuous current control
    float stance_current = 0.1f;      // 스탠스 단계 연속 전류 (A) - 사용자 설정 가능
};

#endif