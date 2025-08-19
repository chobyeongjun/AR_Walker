#ifndef IMU_H
#define IMU_H

#include "Arduino.h"
#include <stdint.h>

class IMU
{
public:
    IMU(uint8_t IMU_id);
    void begin(long baudrate);

    void read();

    // IMU data variables
    float gyro_x = 0;
    float gyro_y = 0;
    float gyro_z = 0;
    float roll = 0;
    float pitch = 0;
    float yaw = 0;
    float acc_x = 0;
    float acc_y = 0;
    float acc_z = 0;


private:
    enum class State
    {
        WAITING_FOR_SOP, // Waiting for the start of the packet
        READING_DATA     // Reading the data packet
    };
    // 체크섬 일치, 데이터 파싱
    float roll_raw, pitch_raw, yaw_raw;
    float gyro_x_raw, gyro_y_raw, gyro_z_raw;
    float acc_x_raw, acc_y_raw, acc_z_raw;
    
    State _state = State::WAITING_FOR_SOP;

    uint8_t IMU_id;
    uint8_t _bytes_read = 0;
    uint8_t _data_buffer[30]; // Buffer for IMU data packets
};

#endif // IMU_H