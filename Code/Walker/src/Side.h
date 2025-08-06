#ifndef SIDE_H_
#define SIDE_H_

// Side 클래스가 의존하는 다른 클래스들의 헤더 파일을 포함해야 합니다.
// (사용자님의 프로젝트 구조에 맞게 경로를 확인해주세요)
#include "ExoData.h" // ExoData 구조체 정의
#include "Joint.h"   // Joint 클래스 정의
#include "Imu.h"     // IMU 클래스 정의

class Side
{
public:
  /**
   * @brief Side 클래스 생성자
   * @param is_left 이 인스턴스가 왼쪽인지 여부
   * @param exo_data 전체 외골격 데이터 구조체에 대한 포인터
   */
  Side(bool is_left, ExoData *exo_data);

  void run_side();

  void read_data();

  void update_motor_cmds();

  void disable_motors();

  void set_controller(int joint, int controller);

  void clear_step_time_estimate();

private:
  // --- 멤버 변수 ---
  bool _is_left;
  ExoData *_exo_data;
  SideData *_side_data;

  Joint _knee;
  Joint _ankle;
  Imu _imu;

  // 보행 감지를 위한 상태 변수
  float _prev_ankle_angle = 0.0f;
  float _current_ankle_angle = 0.0f;
  float _prev_gyro_y = 0.0f;
  float _current_gyro_y = 0.0f;
  bool _was_in_plantarflexion = false;

  // 적응형 임계값을 위한 버퍼 및 관련 변수
  static const int GYRO_BUFFER_SIZE = 50;
  float _gyro_y_buffer[GYRO_BUFFER_SIZE] = {0.0f};
  int _gyro_buffer_index = 0;
  bool _gyro_buffer_filled = false;

  // 동적으로 업데이트되는 두 가지 핵심 임계값
  float _adaptive_gyro_threshold = 0.5f;           // '움직임' 감지용
  float _adaptive_gyro_stillness_threshold = 0.2f; // '정지' 감지용

  // --- Private 도우미 함수 ---

  void _update_gait_state();

  bool _check_imu_heel_off();

  bool _check_imu_toe_off();

  void _update_gait_duration();

  float _calc_percent_gait();

  void _update_adaptive_thresholds();
};

#endif // SIDE_H_