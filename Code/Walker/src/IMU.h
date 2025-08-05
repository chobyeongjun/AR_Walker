#ifndef IMU_H
#define IMU_H

#include "Arduino.h"
#include <stdint.h>

class IMU
{
public:
  IMU();

  // IMU 시리얼 통신을 초기화하는 함수
  void begin(long baudrate);

  // 시리얼에서 바이너리 데이터를 읽어와 파싱하는 함수
  void readData();

  // IMU 데이터 변수
  float acc_x = 0;
  float acc_y = 0;
  float acc_z = 0;
  float gyro_x = 0;
  float gyro_y = 0;
  float gyro_z = 0;
  float roll = 0;
  float pitch = 0;
  float yaw = 0;

private:
  // 바이너리 데이터 수신 상태
  enum class State
  {
    WAITING_FOR_SOP,
    READING_DATA
  };

  State _state = State::WAITING_FOR_SOP;

  uint8_t _bytes_read = 0;
  uint8_t _data_buffer[30]; // IMU 데이터에 필요한 크기로 설정 (예시)
};

#endif // IMU_H