#ifndef Side_h
#define Side_h

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"
#include "joint.h"
#include "controller.h"
#include "IMU.h"
#include "ParseIni.h"
#include "Board.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include <stdint.h>
#include <algorithm>

class Side
{
public:
  Side(bool is_left, ExoData *exo_data);
  void run_side();
  void check_calibration();
  void read_data();
  void update_motor_cmds();
  void clear_step_time_estimate();
  void disable_motors();

private:
  
  // IMU 기반 보행 감지 함수 선언
  bool _check_heel_strike(float ankle_gyro_y, float ankle_accel_z);
  bool _check_heel_off(float ankle_gyro_y, float ankle_pitch);
  
  // 보행 주기 계산 및 업데이트 함수 선언
  float _calc_percent_gait();
  float _update_expected_swing_duration();
  float _calc_percent_swing();

  // Data that can be accessed
  ExoData *_data;
  SideData *_side_data;

  KneeJoint _knee;
  AnkleJoint _ankle;

  bool _is_left;
  
  
  // Heel 이벤트 감지를 위한 상태 플래그
  bool _prev_heel_off_state;
  bool _prev_heel_strike_state;

  static const uint8_t _num_steps_avg = 5;
  unsigned int _swing_times[_num_steps_avg]; // 스윙 시간 저장 배열
  
  unsigned int _heel_strike_timestamp;
  unsigned int _prev_heel_strike_timestamp; 
  unsigned int _heel_off_timestamp;
  unsigned int _prev_heel_off_timestamp;
  unsigned int _expected_swing_duration;

};
#endif
#endif