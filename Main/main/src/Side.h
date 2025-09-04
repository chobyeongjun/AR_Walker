#ifndef Side_h
#define Side_h

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"
#include "Joint.h"
#include "Controller.h"
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
  void read_data();
  void update_motor_cmds();
  void clear_swing_time_estimate();
  void disable_motors();

private:
  
  // IMU 기반 보행 감지 함수 선언
  bool _check_heel_strike(float ankle_gyro_y);
  bool _check_heel_off(float ankle_gyro_y);
  
  // 보행 주기 계산 및 업데이트 함수 선언
  float _calc_percent_gait();
  float _update_expected_swing_duration();
  
  // 타이밍 이벤트 계산 함수
  void _calc_gait_timing_events();
  void _reset_timing_flags();  // 새 보행주기 시작 시 플래그 리셋
  void _calc_assistance_force(); // 타이밍 기반 보조력 계산

  // Data that can be accessed
  ExoData *_data;
  SideData *_side_data;

  KneeJoint _knee;
  AnkleJoint _ankle;

  bool _is_left;
  
  
  // Heel 이벤트 감지를 위한 상태 플래그
  bool _prev_heel_off_state;
  bool _prev_heel_strike_state;

  // IMU 기반 보행 감지 임계값들
  float heel_strike_gyro_threshold;           // heel strike 감지용 gyro 임계값
  float heel_off_gyro_threshold;        // heel off 감지용 gyro 임계값
  
  // 타이밍 이벤트 플래그들
  bool onset_triggered;
  bool peak_triggered;
  bool release_triggered;

  static const uint8_t _num_steps_avg = 50;
  unsigned int _swing_times[_num_steps_avg]; // 스윙 시간 저장 배열
  
  unsigned int _heel_strike_timestamp;
  unsigned int _prev_heel_strike_timestamp; 
  unsigned int _heel_off_timestamp;
  unsigned int _prev_heel_off_timestamp;
  unsigned int _expected_swing_duration;

};
#endif
#endif