#ifndef SideData_h
#define SideData_h

#include "Arduino.h"

#include "JointData.h"
#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>

// Forward declaration
class ExoData;

class SideData
{

public:
    SideData(bool is_left, uint8_t *config_to_send);

    void reconfigure(uint8_t *config_to_send);

    JointData knee;
    JointData ankle;

    float percent_gait;            /**< Estimate of the percent gait based on heel strike */
    float expected_swing_duration; /**< Estimate of how long the next swing will take based on the most recent swing times */

    bool heel_strike;
    bool heel_off;

    // IMU 기반 보행 감지 변수들
    float pitch_threshold; /**< heel off 감지 임계값 */
    float prev_gyro_y;     /**< 이전 gyro_y 값 (zero crossing용) */

    // 타이밍 변수들
    unsigned long heel_off_timestamp;    /**< heel off 발생 시각 */
    unsigned long heel_strike_timestamp; /**< heel strike 발생 시각 */
    unsigned long prev_gait_cycle_time;  /**< 이전 보행 주기 시간 */

    // 예상 지속시간
    float expected_gait_duration; /**< 예상 전체 보행주기 시간 */

    bool is_left; /**< 1 if the side is on the left, 0 otherwise */
    bool is_used; /**< 1 if the side is used, 0 otherwise */

    float ankle_angle_at_ground_strike;
    float expected_duration_window_upper_coeff;
    float expected_duration_window_lower_coeff;

    bool onset_event;
    bool peak_event;
    bool release_event;

    float onset_timing_percent;   
    float peak_timing_percent;    
    float release_timing_percent;
    
    // Controller에서 사용할 힘 설정값들
    float max_reference_force;        /**< 최대 보조 힘 */
    float current_reference_force;    /**< 현재 출력할 힘 */
    bool assistance_active;           /**< 보조 활성화 상태 */
    
    // 보행 상태 판단 함수들 (Joint.cpp에서 사용) - 경계값 안전 처리
    bool is_swing_phase() const { 
        return percent_gait > 0 && percent_gait <= 110 && expected_swing_duration > 0;  // 110% 여유 허용
    }
    bool is_early_swing() const { return percent_gait > 0 && percent_gait < 30; }
    bool is_mid_swing() const { return percent_gait >= 30 && percent_gait <= 70; }
    bool is_late_swing() const { return percent_gait > 70 && percent_gait <= 110; }  // 110% 여유 허용
};

#endif