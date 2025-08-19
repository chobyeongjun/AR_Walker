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
    // **수정**: 생성자 시그니처를 올바르게 선언하고, 인자 이름을 명확하게 변경했습니다.
    JointData(config_defs::joint_id id, uint8_t* config_to_send, float loadcell_bias, float loadcell_sensitive);

    // **수정**: reconfigure 메서드에 로드셀 보정값 매개변수를 추가했습니다.
    void reconfigure(uint8_t *config_to_send, float loadcell_bias, float loadcell_sensitive);

    config_defs::joint_id id;    
    MotorData motor;             
    ControllerData controller;   
    
    float loadcell_reading;      
    float loadcell_bias;         
    float loadcell_sensitive;    

    bool is_left;                 
    bool is_used;                 

    float position;               
    float velocity;              

    // IMU 데이터 필드
    float imu_roll = 0.0f;
    float imu_pitch = 0.0f;
    float imu_yaw = 0.0f;
    float imu_gyro_x = 0.0f;
    float imu_gyro_y = 0.0f;
    float imu_gyro_z = 0.0f;
    float imu_acc_x = 0.0f;
    float imu_acc_y = 0.0f;
    float imu_acc_z = 0.0f;
};

#endif