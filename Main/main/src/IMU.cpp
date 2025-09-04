#include "IMU.h"
#include "Logger.h"
#include <string.h>
#include "Board.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define IMU_SERIAL Serial4

IMU::IMU(uint8_t IMU_id) : IMU_id(IMU_id) {}

void IMU::begin(long baudrate)
{
    IMU_SERIAL.begin(baudrate);
    while (!IMU_SERIAL)
    {
        ;
    }
}

void IMU::calibrate()
{
    this->roll_initial = (float)this->roll_raw / 100.0f;
    this->pitch_initial = (float)this->pitch_raw / 100.0f;
    this->yaw_initial = (float)this->yaw_raw / 100.0f;
    this->is_calibrated = true;
    logger::println("IMU calibrated successfully.");
}

bool IMU::read()
{
    const int TOTAL_BYTES = 2 + 1 + 1 + (3 * 2) + (3 * 2) + (3 * 2) + 2 + 2 + 2;
    static uint8_t packet_data[TOTAL_BYTES]; // 패킷 데이터 버퍼
    static int buffer_index = 0;
    // SOP (Start of Packet) 2 + CH 1 + ID 1 + Angle 3 + Gyro 3 + Acc 3 + Battery 2 + Time 2 + CHK 2;
    const int DATA_LENGTH_FOR_CHECKSUM = TOTAL_BYTES - 2;

    // Serial4에서 데이터 읽기
    while (IMU_SERIAL.available() && buffer_index < TOTAL_BYTES) {
        packet_data[buffer_index++] = IMU_SERIAL.read();
    }

    // 완전한 패킷이 아니면 대기
    if (buffer_index < TOTAL_BYTES) {
        return false;
    }

    if (packet_data[3] == IMU_id)
    {
        uint16_t checksum = 0;
        for (int i = 0; i < DATA_LENGTH_FOR_CHECKSUM; i++)
        {
            checksum += packet_data[i];
        }

        uint16_t received_checksum;
        memcpy(&received_checksum, &packet_data[TOTAL_BYTES - 2], 2);
        uint16_t imu_checksum = __builtin_bswap16(received_checksum);

        if (imu_checksum == checksum)
        {

            static uint16_t prev_time_stamp = 0;
            uint16_t time_stamp_raw = 0;

            uint16_t current_offset = 4;

            memcpy(&roll_raw, &packet_data[current_offset], 2);
            memcpy(&pitch_raw, &packet_data[current_offset + 2], 2);
            memcpy(&yaw_raw, &packet_data[current_offset + 4], 2);
            current_offset += 6;

            roll_raw = __builtin_bswap16(roll_raw);
            pitch_raw = __builtin_bswap16(pitch_raw);
            yaw_raw = __builtin_bswap16(yaw_raw);

            memcpy(&gyro_x_raw, &packet_data[current_offset], 2);
            memcpy(&gyro_y_raw, &packet_data[current_offset + 2], 2);
            memcpy(&gyro_z_raw, &packet_data[current_offset + 4], 2);
            current_offset += 6;

            gyro_x_raw = __builtin_bswap16(gyro_x_raw);
            gyro_y_raw = __builtin_bswap16(gyro_y_raw);
            gyro_z_raw = __builtin_bswap16(gyro_z_raw);

            memcpy(&acc_x_raw, &packet_data[current_offset], 2);
            memcpy(&acc_y_raw, &packet_data[current_offset + 2], 2);
            memcpy(&acc_z_raw, &packet_data[current_offset + 4], 2);
            current_offset += 6;

            acc_x_raw = __builtin_bswap16(acc_x_raw);
            acc_y_raw = __builtin_bswap16(acc_y_raw);
            acc_z_raw = __builtin_bswap16(acc_z_raw);

            memcpy(&battery_raw, &packet_data[current_offset], 2);
            battery_raw = __builtin_bswap16(battery_raw);

            memcpy(&time_stamp_raw, &packet_data[current_offset + 2], 2);
            time_stamp_raw = __builtin_bswap16(time_stamp_raw);

            if (!is_calibrated)
            {
                roll_initial = (float)roll_raw / 100.0f;
                pitch_initial = (float)pitch_raw / 100.0f;
                yaw_initial = (float)yaw_raw / 100.0f;
                is_calibrated = true;

                logger::print("IMU ID ");
                logger::print(IMU_id);
                logger::println(": Auto-calibration complete.");
            }

            roll = (roll_raw / 100.0f) - roll_initial;
            pitch = (pitch_raw / 100.0f) - pitch_initial;
            yaw = (yaw_raw / 100.0f) - yaw_initial;

            gyro_x = gyro_x_raw / 10.0f;
            gyro_y = gyro_y_raw / 10.0f;
            gyro_z = gyro_z_raw / 10.0f;
            acc_x = acc_x_raw / 1000.0f;
            acc_y = acc_y_raw / 1000.0f;
            acc_z = acc_z_raw / 1000.0f;
            battery = battery_raw;
            time_stamp = time_stamp_raw;

            if (prev_time_stamp != 0)
            {
                if (time_stamp_raw < prev_time_stamp && !(time_stamp_raw < 500 && prev_time_stamp > 59500))
                {
                    logger::println("IMU Error : Time Error");
                    time_error_detected = true;  // Error 플래그 설정
                    return false;
                }
            }
            
            // 정상적인 데이터 수신 시 에러 플래그 리셋
            time_error_detected = false;
            prev_time_stamp = time_stamp_raw;
            return true;
        }
        else
        {
            logger::print("Checksum mismatch!");
        }
           // 패킷 처리 후 버퍼 리셋
        buffer_index = 0;
    }
    return false;
}

// 필요한 getter 함수들만 구현
uint8_t IMU::get_battery() const { return battery; }
uint16_t IMU::get_time_stamp() const { return time_stamp; }
uint8_t IMU::get_imu_id() const { return IMU_id; }

#endif // defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)