#include "IMU.h"
#include "Logger.h"
#include <string.h>
#include "Board.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define IMU_SERIAL Serial4
#endif

IMU::IMU() {}

void IMU::begin(long baudrate, uint8_t id)
{
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    // Board.h에 정의된 핀 정보를 사용해 시리얼 통신을 시작
    IMU_SERIAL.begin(baudrate, SERIAL_8N1, logic_micro_pins::imu_tx_pin, logic_micro_pins::imu_rx_pin);

    // `<id posz>` 명령 생성 // postiion, velocity 초기화 및 전송 imu안되면 여기 의심해보기
    char cmd[8];
    sprintf(cmd, "<%02dposz>", id);
    IMU_SERIAL.write(cmd, strlen(cmd));
    delay(100); // IMU 응답 대기
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

            // 데이터 패킷 구성
            // SOP(2) + CH(1) + ID(1) +
            // Distance Global은 3항목, Euler Angle은 3항목, Gyroscope는 3항목
            // CHK(2)
            const int TOTAL_BYTES = 2 + 1 + 1 + (3 * 2) + (3 * 2) + (3 * 2) + 2;
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
                    int16_t dist_x_raw, dist_y_raw, dist_z_raw;

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

                    // 거리 (Global)
                    memcpy(&dist_x_raw, &_data_buffer[current_offset], 2);
                    memcpy(&dist_y_raw, &_data_buffer[current_offset + 2], 2);
                    memcpy(&dist_z_raw, &_data_buffer[current_offset + 4], 2);

                    // 스펙시트에 따라 각 항목을 변환
                    roll = (float)roll_raw / 100.0f; // Euler Angle은 100을 나눔
                    pitch = (float)pitch_raw / 100.0f;
                    yaw = (float)yaw_raw / 100.0f;

                    gyro_x = (float)gyro_x_raw / 10.0f; // Gyro는 10을 나눔
                    gyro_y = (float)gyro_y_raw / 10.0f;
                    gyro_z = (float)gyro_z_raw / 10.0f;

                    dist_x = (float)dist_x_raw / 1000.0f; // 스펙시트()에 따라 1000을 나눔
                    dist_y = (float)dist_y_raw / 1000.0f;
                    dist_z = (float)dist_z_raw / 1000.0f;
                }
                else
                {
                    logger::println("Checksum mismatch!", LogLevel::Error);
                }

                _bytes_read = 0;
                _state = State::WAITING_FOR_SOP;
            }
            break;
        }
    }
#endif
}