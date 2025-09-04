#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include "Board.h"
#include "Logger.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

class IMU {
public:
    // 생성자
    IMU(uint8_t IMU_id);

    // 초기화 함수
    void begin(long baudrate);

    // 패킷 읽기 및 처리 함수
    bool read_packet(uint8_t* packet_data, size_t packet_size);

    // 캘리브레이션 함수
    void calibrate();

    // 💡 public 멤버 변수들
    float roll, pitch, yaw;
    float gyro_x, gyro_y, gyro_z;
    float acc_x, acc_y, acc_z;
    float battery; 
    
    uint8_t IMU_id;
    bool is_calibrated = false;

private:
    // 💡 private 멤버 변수들
    int16_t roll_raw, pitch_raw, yaw_raw;
    int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
    int16_t acc_x_raw, acc_y_raw, acc_z_raw;
    int16_t battery_raw; 

    // 초기 보정값
    float roll_initial = 0;
    float pitch_initial = 0;
    float yaw_initial = 0;

    
};

#endif // defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#endif // IMU_H