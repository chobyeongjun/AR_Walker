#ifndef ERROR_CODES_H
#define ERROR_CODES_H

enum ErrorCodes : int
{
    NO_ERROR = 0,                    // This must be the first element
    
    // HIGH PRIORITY - System Safety Critical
    MOTOR_TIMEOUT_ERROR,             // 모터 통신 타임아웃 (CAN 버스 통신 실패 포함)
    IMU_ERROR,                       // IMU 센서 오류
    LOADCELL_SENSOR_ERROR,           // 로드셀 센서 오류
    EMERGENCY_STOP_ERROR,            // 비상정지
    
    // MEDIUM PRIORITY - Functional Issues  
    FORCE_SENSOR_RANGE_ERROR,        // 힘 센서 범위 초과
    BATTERY_LOW_ERROR,               // 배터리 부족
    MOTOR_CURRENT_OVERLOAD_ERROR,    // 모터 전류 초과
    MOTOR_TEMPERATURE_ERROR,         // 모터 온도 위험 (110°C 이상)
    MOTOR_TEMPERATURE_WARNING,       // 모터 온도 경고 (100°C 이상)
    IMU_BATTERY_LOW_ERROR,           // IMU 배터리 위험 (5% 이하)
    IMU_BATTERY_WARNING,             // IMU 배터리 경고 (10% 이하)
    MOTOR_CABLE_ERROR,               // 모터 케이블 연결 오류
    
    // LOW PRIORITY - Debug/Test
    TEST_ERROR,                      // 테스트용 (기존)
    ERROR_CODE_LENGTH                // This must be the last element
};

#endif // ERROR_CODES_H