#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

  #include "src/Board.h"
  #include "src/Utilities.h"
  #include "src/ParseIni.h"
  #include "src/IMU.h"
  #include "src/Logger.h"

  // 테스트를 위한 IMU 객체 (ID 0)
  IMU test_imu(0);
  
  void setup()
  {
    Serial.begin(115200);
    while(!Serial)
    {
      ;
    }
    // IMU 시리얼 통신 시작
    test_imu.begin(115200);

    // 시리얼 모니터 헤더 출력
    logger::print("IMU ID\tRoll\tPitch\tYaw\tGyroX\tGyroY\tGyroZ\tAccelX\tAccelY\tAccelZ\n");
  }
  
  void loop()
  {
    // 데이터 수집 주기 제어
    static int state_period_ms = 10;
    static int last_transition_time = millis();
    int current_time = millis();
    
    if ((current_time - last_transition_time) >= state_period_ms)
    {
      // IMU 데이터 읽기
      test_imu.read();

      // 시리얼 모니터에 IMU 데이터 출력
      logger::print(test_imu.IMU_id);
      logger::print("\t");
      logger::print(test_imu.roll);
      logger::print("\t");
      logger::print(test_imu.pitch);
      logger::print("\t");
      logger::print(test_imu.yaw);
      logger::print("\t");
      logger::print(test_imu.gyro_x);
      logger::print("\t");
      logger::print(test_imu.gyro_y);
      logger::print("\t");
      logger::print(test_imu.gyro_z);
      logger::print("\t");
      logger::print(test_imu.acc_x);
      logger::print("\t");
      logger::print(test_imu.acc_y);
      logger::print("\t");
      logger::print(test_imu.acc_z);
      logger::print("\n");
      
      last_transition_time = current_time;
    }
  }
#endif