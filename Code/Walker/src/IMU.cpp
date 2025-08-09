#include "IMU.h"
#include "Logger.h"
#include <string.h>
#include "Board.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define IMU_SERIAL Serial4

IMU::IMU(uint8_t IMU_id) : IMU_id(IMU_id)
{
}

void IMU::begin(long baudrate)
{
    // Board.h에 정의된 핀 정보를 사용해 시리얼 통신을 시작합니다.
    IMU_SERIAL.begin(baudrate, SERIAL_8N1, logic_micro_pins::imu_tx_pin, logic_micro_pins::imu_rx_pin);

    // IMU의 위치(Yaw)를 0으로 초기화하는 명령을 전송합니다.
    char cmd[8];
    sprintf(cmd, "<%02dposz>", IMU_id);
    IMU_SERIAL.write(cmd, strlen(cmd));
    delay(100); // IMU가 명령에 응답할 시간을 기다립니다.

    // AVC (Active Vibration Cancellation) 기능을 활성화하는 명령을 전송합니다.
    // data '1'은 AVC ON을 의미합니다.
    char cmd_avc[10];
    sprintf(cmd_avc, "<%02davca_e1>", IMU_id);
    IMU_SERIAL.write(cmd_avc, strlen(cmd_avc));
    delay(100);

    logger::print("IMU ");
    logger::print(IMU_id);
    logger::println(" initialized, position zeroed, and AVC enabled.");
}

void IMU::read()
{
    while (IMU_SERIAL.available())
    {
        uint8_t byte = IMU_SERIAL.read();

        switch (_state)
        {
        case State::WAITING_FOR_SOP:
            // 패킷의 시작(Start of Packet, SOP)인 0x55 0x55를 수신 대기합니다.
            if (byte == 0x55 && IMU_SERIAL.available() > 0 && IMU_SERIAL.peek() == 0x55)
            {
                _data_buffer[_bytes_read++] = byte;
                byte = IMU_SERIAL.read(); // 다음 바이트도 읽어서 버퍼에 저장
                _data_buffer[_bytes_read++] = byte;
                _state = State::READING_DATA;
            }
            break;

        case State::READING_DATA:
            _data_buffer[_bytes_read++] = byte;

            // IMU 데이터 패킷의 총 바이트 수.
            // SOP(2) + CH(1) + ID(1) + Euler Angle(3*2) + Gyroscope(3*2) + Checksum(2)
            const int TOTAL_BYTES = 2 + 1 + 1 + (3 * 2) + (3 * 2) + 2;
            const int DATA_START_INDEX = 4; // SOP(2), CH(1), ID(1) 이후 데이터 시작 위치

            if (_bytes_read >= TOTAL_BYTES)
            {
                if (_data_buffer[3] == _id)
                {
                    // 체크섬 계산 (SOP 포함)
                    uint16_t checksum = 0;
                    for (int i = 0; i < TOTAL_BYTES - 2; i++)
                    {
                        checksum += _data_buffer[i];
                    }

                    // 수신된 체크섬 값
                    uint16_t received_checksum;
                    memcpy(&received_checksum, &_data_buffer[TOTAL_BYTES - 2], 2);

                    if (received_checksum == checksum)
                    {
                        // 체크섬 일치, 데이터 파싱
                        float roll_raw, pitch_raw, yaw_raw;
                        float gyro_x_raw, gyro_y_raw, gyro_z_raw;

                        uint16_t current_offset = DATA_START_INDEX;

                        // 오일러 각도 (Roll, Pitch, Yaw)를 바이트 버퍼에서 읽어와 변수에 저장
                        memcpy(&roll_raw, &_data_buffer[current_offset], 2);
                        memcpy(&pitch_raw, &_data_buffer[current_offset + 2], 2);
                        memcpy(&yaw_raw, &_data_buffer[current_offset + 4], 2);
                        current_offset += 6;

                        // 자이로스코프 (X, Y, Z)를 바이트 버퍼에서 읽어와 변수에 저장
                        memcpy(&gyro_x_raw, &_data_buffer[current_offset], 2);
                        memcpy(&gyro_y_raw, &_data_buffer[current_offset + 2], 2);
                        memcpy(&gyro_z_raw, &_data_buffer[current_offset + 4], 2);

                        // 스펙시트에 따라 raw 데이터를 실제 값으로 변환
                        roll = roll_raw / 100.0f;
                        pitch = pitch_raw / 100.0f;
                        yaw = yaw_raw / 100.0f;

                        gyro_x = gyro_x_raw / 10.0f;
                        gyro_y = gyro_y_raw / 10.0f;
                        gyro_z = gyro_z_raw / 10.0f;
                    }
                    else
                    {
                        logger::println("Checksum mismatch!", LogLevel::Error);
                    }

                    // 패킷 처리가 완료되면 상태를 초기화
                    _bytes_read = 0;
                    _state = State::WAITING_FOR_SOP;
                }
                break;
            }
        }
    }
#endif // defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)