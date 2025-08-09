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

private:
    enum class State
    {
        WAITING_FOR_SOP, // Waiting for the start of the packet
        READING_DATA     // Reading the data packet
    };

    State _state = State::WAITING_FOR_SOP;

    uint8_t IMU_id;
    uint8_t _bytes_read = 0;
    uint8_t _data_buffer[24]; // Buffer for IMU data packets
};

#endif // IMU_H