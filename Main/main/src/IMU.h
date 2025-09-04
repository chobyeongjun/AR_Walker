#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include "Board.h"
#include "Logger.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

class IMU
{
public:
    // 생성자
    IMU(uint8_t IMU_id);

    // 초기화 함수
    void begin(long baudrate);

    // Serial4에서 데이터를 읽어 처리하는 함수
    bool read();

    // 캘리브레이션 함수
    void calibrate();

    float roll, pitch, yaw;
    float gyro_x, gyro_y, gyro_z;
    float acc_x, acc_y, acc_z;
    uint8_t battery;
    uint16_t time_stamp;

    uint8_t get_battery() const;
    uint16_t get_time_stamp() const;
    uint8_t get_imu_id() const;

    uint8_t IMU_id;
    bool is_calibrated = false;
    bool time_error_detected = false;  // 시간 연속성 오류 플래그

private:
   
    uint16_t prev_time_stamp;

    // 💡 private 멤버 변수들
    int16_t roll_raw, pitch_raw, yaw_raw;
    int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
    int16_t acc_x_raw, acc_y_raw, acc_z_raw;
    uint16_t battery_raw;
    uint16_t time_stamp_raw;

    // 초기 보정값
    float roll_initial = 0;
    float pitch_initial = 0;
    float yaw_initial = 0;
};

#endif // defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#endif // IMU_H