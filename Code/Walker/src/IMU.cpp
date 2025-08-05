#include "IMU.h"
#include "Logger.h"
#include <string.h>
#include "Board.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define IMU_SERIAL Serial4
#endif

IMU::IMU() {}

void IMU::begin(long baudrate)
{
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    // Board.h에 정의된 핀 정보를 사용해 시리얼 통신을 시작
    IMU_SERIAL.begin(baudrate, SERIAL_8N1, logic_micro_pins::imu_tx_pin, logic_micro_pins::imu_rx_pin);
#endif
}

void IMU::readData()
{
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    while (IMU_SERIAL.available())
    {
        uint8_t byte = IMU_SERIAL.read();

        switch (_state)
        {
        case State::WAITING_FOR_SOP:
            // SOP (Start of Packet) 55 55 (HEX) 수신 대기
            if (byte == 0x55 && IMU_SERIAL.available() > 0 && IMU_SERIAL.peek() == 0x55)
            {
                _data_buffer[_bytes_read++] = byte;
                byte = IMU_SERIAL.read();
                _data_buffer[_bytes_read++] = byte;
                _state = State::READING_DATA;
            }
            break;

        case State::READING_DATA:
            _data_buffer[_bytes_read++] = byte;

            // 데이터 패킷 구성 (총 26바이트)
            // SOP(2) + CH(1) + ID(1) +
            // EulerAngle(Roll,Pitch,Yaw) 3*2바이트 = 6바이트 +
            // Gyro(X,Y,Z) 3*2바이트 = 6바이트 +
            // Linear Accel(Global) 3*2바이트 = 6바이트 +
            // Velocity(Global) 3*2바이트 = 6바이트 +
            // CHK(2)
            const int TOTAL_BYTES = 2 + 1 + 1 + (3 * 2) + (3 * 2) + (3 * 2) + (3 * 2) + 2;
            const int DATA_START_INDEX = 4; // SOP(2), CH(1), ID(1) 이후 데이터 시작

            if (_bytes_read >= TOTAL_BYTES)
            {
                // 체크섬 계산 (SOP 포함, overflow 무시)
                uint16_t checksum = 0;
                for (int i = 0; i < TOTAL_BYTES - 2; i++)
                {
                    checksum += _data_buffer[i];
                }

                uint16_t received_checksum;
                memcpy(&received_checksum, &_data_buffer[TOTAL_BYTES - 2], 2);

                if (received_checksum == checksum)
                {
                    // 체크섬 일치, 데이터 파싱
                    int16_t roll_raw, pitch_raw, yaw_raw;
                    int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;
                    int16_t lin_acc_x_raw, lin_acc_y_raw, lin_acc_z_raw;
                    int16_t vel_x_raw, vel_y_raw, vel_z_raw;

                    int current_offset = DATA_START_INDEX;

                    // 오일러 각도 (Roll, Pitch, Yaw)
                    memcpy(&roll_raw, &_data_buffer[current_offset], 2);
                    memcpy(&pitch_raw, &_data_buffer[current_offset + 2], 2);
                    memcpy(&yaw_raw, &_data_buffer[current_offset + 4], 2);
                    current_offset += 6;

                    // 자이로스코프 (X, Y, Z)
                    memcpy(&gyro_x_raw, &_data_buffer[current_offset], 2);
                    memcpy(&gyro_y_raw, &_data_buffer[current_offset + 2], 2);
                    memcpy(&gyro_z_raw, &_data_buffer[current_offset + 4], 2);
                    current_offset += 6;

                    // 선형 가속도 (Global)
                    memcpy(&lin_acc_x_raw, &_data_buffer[current_offset], 2);
                    memcpy(&lin_acc_y_raw, &_data_buffer[current_offset + 2], 2);
                    memcpy(&lin_acc_z_raw, &_data_buffer[current_offset + 4], 2);
                    current_offset += 6;

                    // 속도 (Global)
                    memcpy(&vel_x_raw, &_data_buffer[current_offset], 2);
                    memcpy(&vel_y_raw, &_data_buffer[current_offset + 2], 2);
                    memcpy(&vel_z_raw, &_data_buffer[current_offset + 4], 2);

                    // 스펙시트에 따라 각 항목을 변환
                    roll = (float)roll_raw / 100.0f; // Euler Angle은 100을 나눔
                    pitch = (float)pitch_raw / 100.0f;
                    yaw = (float)yaw_raw / 100.0f;

                    gyro_x = (float)gyro_x_raw / 10.0f; // Gyro는 10을 나눔
                    gyro_y = (float)gyro_y_raw / 10.0f;
                    gyro_z = (float)gyro_z_raw / 10.0f;

                    acc_x = (float)lin_acc_x_raw / 1000.0f; // 가속도는 1000을 나눔
                    acc_y = (float)lin_acc_y_raw / 1000.0f;
                    acc_z = (float)lin_acc_z_raw / 1000.0f;

                    vel_x = (float)vel_x_raw / 1000.0f; // 속도도 1000을 나눔
                    vel_y = (float)vel_y_raw / 1000.0f;
                    vel_z = (float)vel_z_raw / 1000.0f;
                }

                _bytes_read = 0;
                _state = State::WAITING_FOR_SOP;
            }
            break;
        }
    }
#endif
}