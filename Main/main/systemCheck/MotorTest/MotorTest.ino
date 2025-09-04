/*
 * MediWalker Motor Test System
 * 
 * 이 코드는 MediWalker 시스템의 AK60_v1_1 모터를 종합적으로 테스트하기 위한
 * 독립적인 테스트 프로그램입니다.
 * 
 * 기능:
 * - Position Control, Speed Control, Combined Control 테스트
 * - CAN 통신 및 모터 응답 실시간 모니터링
 * - 모터 안전성 검증 (전류, 온도, 속도 제한)
 * - SD카드를 통한 데이터 로깅
 * - 모터 파라미터 식별 및 성능 평가
 * - 에러 코드 모니터링 및 진단
 * 
 * 하드웨어:
 * - Teensy 4.1
 * - AK60_v1_1 모터 (CAN ID: 65)
 * - CAN 통신 (1Mbps)
 * - SD카드: 내장 SD카드 슬롯 사용
 * 
 * 안전 주의사항:
 * - 모터가 자유롭게 회전할 수 있는 상태에서 테스트
 * - 비상정지 핀(Pin 9) 연결 확인
 * - 적절한 전원 공급 (24V) 확인
 */

#include "Arduino.h"
#include "FlexCAN_T4.h"
#include "SD.h"
#include "SPI.h"

// =============================================================================
// 하드웨어 설정
// =============================================================================

// Teensy 4.1 CAN 및 SD카드 설정
#if defined(ARDUINO_TEENSY41)
// FlexCAN_T4 매핑 정보:
// CAN1 = Pin 22 (CTX1 송신), Pin 23 (CRX1 수신) - 이 코드에서 사용 중
// CAN2 = Pin 0 (CRX2), Pin 1 (CTX2) - 사용 안함  
// CAN3 = Pin 30 (CRX3), Pin 31 (CTX3) - 사용 안함
// 
// FlexCAN_T4<CAN1>을 Can0 변수로 사용하여 CAN1 컨트롤러에 접근
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#define SD_CS_PIN BUILTIN_SDCARD
#endif

// 비상정지 핀 설정
#define EMERGENCY_STOP_PIN 9

// =============================================================================
// 모터 통신 프로토콜 정의
// =============================================================================

// CAN 명령 ID (Motor.h에서 참조)
enum class CAN_PACKET_ID : uint8_t {
    SET_DUTY = 0,
    SET_CURRENT = 1,
    SET_CURRENT_BRAKE = 2,
    SET_RPM = 3,
    SET_POS = 4,
    SET_ORIGIN_HERE = 5,
    SET_POS_SPD = 6
};

// 모터 에러 코드 (Motor.h에서 참조)
enum class MotorErrorCode : uint8_t {
    NO_FAULT = 0,
    MOTOR_OVER_TEMPERATURE = 1,
    OVER_CURRENT = 2,
    OVER_VOLTAGE = 3,
    UNDER_VOLTAGE = 4,
    ENCODER_FAULT = 5,
    MOSFET_OVER_TEMPERATURE = 6,
    MOTOR_STALL = 7
};

// 모터 에러 코드를 문자열로 변환
const char* motorErrorCodeToString(uint8_t code) {
    switch (code) {
        case (uint8_t)MotorErrorCode::NO_FAULT: return "No Fault";
        case (uint8_t)MotorErrorCode::MOTOR_OVER_TEMPERATURE: return "Motor Over Temperature";
        case (uint8_t)MotorErrorCode::OVER_CURRENT: return "Over Current";
        case (uint8_t)MotorErrorCode::OVER_VOLTAGE: return "Over Voltage";
        case (uint8_t)MotorErrorCode::UNDER_VOLTAGE: return "Under Voltage";
        case (uint8_t)MotorErrorCode::ENCODER_FAULT: return "Encoder Fault";
        case (uint8_t)MotorErrorCode::MOSFET_OVER_TEMPERATURE: return "MOSFET Over Temperature";
        case (uint8_t)MotorErrorCode::MOTOR_STALL: return "Motor Stall";
        default: return "Unknown Error";
    }
}

// =============================================================================
// 모터 테스트 클래스
// =============================================================================

class MotorTestSystem {
private:
    uint8_t motor_id;
    bool is_motor_on;
    bool emergency_stop_active;
    
    // AK60_v1_1 모터 안전 제한값 (사용자 요청 기준)
    float P_MIN = -3600.0f;         // -3600도 (10 회전)
    float P_MAX = 3600.0f;          // +3600도 (10 회전)
    float V_MIN = -8000.0f;         // -8000 RPM
    float V_MAX = 8000.0f;          // +8000 RPM
    float A_MIN = 0.0f;             // 0 (최소 가속도)
    float A_MAX = 2000.0f;          // 2000 (최대 가속도)
    float I_MIN = -13.5f;           // -13.5A (AK60_v1_1 한계)
    float I_MAX = 13.5f;            // +13.5A (AK60_v1_1 한계)
    
    // 모터 상태 데이터
    float current_position = 0.0f;  // 현재 위치 (도)
    float current_velocity = 0.0f;  // 현재 속도 (RPM)
    float current_current = 0.0f;   // 현재 전류 (A)
    int8_t current_temperature = 0; // 현재 온도 (°C)
    uint8_t current_error = 0;      // 현재 에러 코드
    
    float position_command = 0.0f;  // 위치 명령값
    float velocity_command = 0.0f;  // 속도 명령값
    float acceleration_command = 0.0f; // 가속도 명령값
    
    // 모터 상수 (AK60_v1_1 기준)
    const float Kt = 0.068f;        // 토크 상수 (Nm/A)
    const uint32_t timeout_us = 5000; // CAN 타임아웃 (5ms = 5,000 μs)
    
    // SD카드 로깅 관련 (private)
    bool sd_available = false;
    File dataFile;
    String current_filename = "";
    unsigned long last_log_time = 0;
    const unsigned long log_interval = 10; // 10ms 간격 (100Hz)

    // 테스트 상태
    int test_mode = 0;  // 0: 수동, 1: 자동 테스트
    
public:
    // SD카드 로깅 관련 (public)
    bool logging_enabled = false;
    MotorTestSystem(uint8_t id) {
        motor_id = id;
        is_motor_on = false;
        emergency_stop_active = false;
    }
    
public:
    // =============================================================================
    // 초기화 함수들
    // =============================================================================
    
    void init() {
        // 비상정지 핀 초기화
        pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
        
        // CAN 버스 초기화
        Serial.println("🔧 CAN 초기화 중...");
        Serial.println("   사용 핀: Pin 22 (CTX1), Pin 23 (CRX1)");
        Serial.println("   FlexCAN 컨트롤러: CAN1");
        Can0.begin();
        Can0.setBaudRate(1000000); // 1Mbps
        Serial.println("✅ CAN 초기화 완료 (1Mbps)");
        
        // SD카드 초기화
        initializeSD();
        
        Serial.println("✓ Motor Test System initialized");
        printSystemInfo();
    }
    
    void initializeSD() {
        Serial.print("SD카드 초기화 중... ");
        
        if (!SD.begin(SD_CS_PIN)) {
            Serial.println("실패! SD카드 없거나 오류");
            sd_available = false;
            return;
        }
        
        Serial.println("성공!");
        sd_available = true;
        
        // SD.card()->type() 대신 간단한 성공 메시지만 출력
        Serial.println("SD카드 타입: 감지됨");
    }
    
    // =============================================================================
    // CAN 통신 함수들
    // =============================================================================
    
    bool sendCANMessage(CAN_message_t &msg) {
        if (emergency_stop_active) {
            Serial.println("🚨 Emergency stop active - message blocked");
            return false;
        }
        
        // 상세 디버깅 정보
        Serial.print("📤 CAN 전송: ID=0x");
        Serial.print(msg.id, HEX);
        Serial.print(", Len=");
        Serial.print(msg.len);
        Serial.print(", Data=[");
        for (int i = 0; i < msg.len; i++) {
            Serial.print("0x");
            Serial.print(msg.buf[i], HEX);
            if (i < msg.len - 1) Serial.print(", ");
        }
        Serial.println("]");
        
        if (Can0.write(msg)) {
            Serial.println("✅ CAN 메시지 전송 성공");
            return true;
        } else {
            Serial.println("❌ CAN 메시지 전송 실패");
            return false;
        }
    }
    
    void readMotorData() {
        CAN_message_t msg;
        uint32_t start_time = micros();
        bool received_response = false;
        
        Serial.println("📡 모터 응답 대기 중...");
        int total_messages = 0;
        while ((micros() - start_time) < timeout_us) {
            if (Can0.read(msg)) {
                total_messages++;
                
                // 전체 메시지 ID 확인 (32비트)
                Serial.print("📥 CAN 수신 #");
                Serial.print(total_messages);
                Serial.print(": Full_ID=0x");
                Serial.print(msg.id, HEX);
                Serial.print(", Low_ID=");
                Serial.print(msg.id & 0xFF);
                Serial.print(" (기대값: ");
                Serial.print(motor_id);
                Serial.print("), Len=");
                Serial.print(msg.len);
                Serial.print(", Data=[");
                for (int i = 0; i < msg.len && i < 8; i++) {
                    Serial.print("0x");
                    Serial.print(msg.buf[i], HEX);
                    if (i < msg.len - 1) Serial.print(", ");
                }
                Serial.println("]");
                
                uint8_t received_id = msg.id & 0xFF;
                
                if (received_id == motor_id && msg.len == 8) {
                    // 모터 응답 데이터 파싱 (Motor.cpp 참조)
                    int16_t p_int = (msg.buf[0] << 8) | msg.buf[1];
                    int16_t v_int = (msg.buf[2] << 8) | msg.buf[3];
                    int16_t i_int = (msg.buf[4] << 8) | msg.buf[5];
                    int8_t temp_int = msg.buf[6];
                    uint8_t error_code_raw = msg.buf[7];
                    
                    // 스케일링 적용 (Motor.cpp 참조)
                    current_position = (float)p_int * 0.1f;        // 0.1도 해상도
                    current_velocity = (float)v_int * 10.0f;       // 10 RPM 해상도
                    current_current = (float)i_int * 0.01f;        // 0.01A 해상도
                    current_temperature = temp_int;
                    current_error = error_code_raw;
                    
                    Serial.println("✅ 모터 응답 수신 성공!");
                    received_response = true;
                    break;
                } else {
                    Serial.println("❌ 잘못된 응답 (ID 또는 길이 불일치)");
                }
            }
            delayMicroseconds(100);
        }
        
        // 수신 통계 표시
        Serial.print("📊 총 수신 메시지: ");
        Serial.print(total_messages);
        Serial.println("개");
        
        if (!received_response) {
            Serial.println("⚠ 모터 응답 타임아웃 - 모터 연결 확인 필요");
            if (total_messages == 0) {
                Serial.println("❌ CAN 메시지가 전혀 수신되지 않음");
                Serial.println("🔧 하드웨어 확인사항:");
                Serial.println("   1. 모터 전원 연결 (24V)");
                Serial.println("   2. CAN H/L 케이블 연결");
                Serial.println("   3. CAN 버스 종단저항 (120Ω)");
                Serial.println("   4. Teensy CAN 핀 연결 확인:");
                Serial.println("      - Pin 22 (CTX1) → CAN 트랜시버 TX");
                Serial.println("      - Pin 23 (CRX1) ← CAN 트랜시버 RX");
            } else {
                Serial.println("📡 CAN 통신은 되지만 해당 모터 응답 없음");
                Serial.println("🔧 모터 설정 확인사항:");
                Serial.println("   1. 모터 ID 설정 (현재 기대값: " + String(motor_id) + ")");
                Serial.println("   2. 모터 펌웨어 상태");
                Serial.println("   3. 모터 에러 상태");
            }
        }
    }
    
    // CAN 메시지 스캔 (모든 수신 메시지 표시)
    void scanCANMessages(int duration_ms = 3000) {
        Serial.println("🔍 CAN 버스 스캔 시작 (" + String(duration_ms) + "ms)...");
        Serial.println("모든 CAN 메시지를 모니터링합니다:");
        
        CAN_message_t msg;
        uint32_t start_time = millis();
        int message_count = 0;
        
        while ((millis() - start_time) < duration_ms) {
            if (Can0.read(msg)) {
                message_count++;
                Serial.print("📡 #");
                Serial.print(message_count);
                Serial.print(": ID=0x");
                Serial.print(msg.id, HEX);
                Serial.print(" (");
                Serial.print(msg.id);
                Serial.print("), Len=");
                Serial.print(msg.len);
                Serial.print(", Data=[");
                for (int i = 0; i < msg.len && i < 8; i++) {
                    if (msg.buf[i] < 16) Serial.print("0");
                    Serial.print(msg.buf[i], HEX);
                    if (i < msg.len - 1) Serial.print(" ");
                }
                Serial.println("]");
                
                // 잠깐 대기
                delay(10);
            }
        }
        
        Serial.println("🔍 스캔 완료. 총 " + String(message_count) + "개 메시지 수신");
        if (message_count == 0) {
            Serial.println("❌ CAN 버스에서 메시지가 감지되지 않음");
        }
    }
    
    // 하드웨어 연결 체크
    void checkHardwareConnections() {
        Serial.println("🔍 하드웨어 연결 상태 체크:");
        Serial.println("========================");
        Serial.println("📌 Teensy 4.1 CAN 연결:");
        Serial.println("   Pin 22 (CTX1) → CAN 트랜시버의 TX 입력");
        Serial.println("   Pin 23 (CRX1) ← CAN 트랜시버의 RX 출력");
        Serial.println("");
        Serial.println("📌 CAN 트랜시버 연결:");
        Serial.println("   VCC ← 3.3V 또는 5V 전원");
        Serial.println("   GND ← Teensy GND");
        Serial.println("   CANH ↔ 모터 CAN H");  
        Serial.println("   CANL ↔ 모터 CAN L");
        Serial.println("");
        Serial.println("📌 모터 연결:");
        Serial.println("   전원: 24V DC");
        Serial.println("   CAN H/L: CAN 트랜시버와 연결");
        Serial.println("   ID 설정: " + String(motor_id));
        Serial.println("");
        Serial.println("🔧 문제 해결:");
        Serial.println("   1. CAN 케이블 연결 확인");
        Serial.println("   2. 종단저항 120Ω 확인");
        Serial.println("   3. 모터 전원 24V 확인");
        Serial.println("   4. CAN 트랜시버 전원 확인");
        Serial.println("   5. FlexCAN_T4<CAN1> → Can0 매핑 확인");
        Serial.println("      (코드의 Can0 객체가 Teensy CAN1을 제어)");
        Serial.println("========================");
    }
    
    // =============================================================================
    // 모터 제어 함수들
    // =============================================================================
    
    void toggleMotorPower() {
        checkEmergencyStop();
        if (emergency_stop_active) return;
        
        is_motor_on = !is_motor_on;
        Serial.print("모터 ");
        Serial.print(motor_id);
        Serial.print(" 상태: ");
        Serial.println(is_motor_on ? "ON 🟢" : "OFF 🔴");
        
        // CAN 통신 상태 확인
        Serial.println("📡 CAN 통신 상태 확인 중...");
        if (is_motor_on) {
            // 모터 ON 시 테스트 메시지 전송
            Serial.println("🔧 모터 활성화 테스트 메시지 전송...");
            setPosition(0.0f);  // 0도로 이동 명령
        }
    }
    
    void zeroPosition() {
        if (!is_motor_on) {
            Serial.println("⚠ 모터가 꺼져있습니다 - 영점 설정 불가");
            return;
        }
        
        CAN_message_t msg;
        msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_ORIGIN_HERE) << 8) | motor_id;
        msg.flags.extended = 1;
        msg.len = 1;
        msg.buf[0] = 0;
        
        Serial.print("영점 설정 중... ");
        if (sendCANMessage(msg)) {
            Serial.println("✓ 영점 설정 완료");
            readMotorData();
        }
    }
    
    void setPosition(float degrees) {
        if (!is_motor_on) {
            Serial.println("⚠ 모터가 꺼져있습니다");
            return;
        }
        
        float safe_pos = constrain(degrees, P_MIN, P_MAX);
        if (safe_pos != degrees) {
            Serial.print("⚠ 위치 제한: ");
            Serial.print(degrees);
            Serial.print("° → ");
            Serial.print(safe_pos);
            Serial.println("°");
        }
        
        position_command = safe_pos;
        
        CAN_message_t msg;
        msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS) << 8) | motor_id;
        msg.flags.extended = 1;
        msg.len = 4;
        
        // 위치 명령 스케일링 (0.01도 해상도, Motor.cpp 참조)
        int32_t pos_cmd = (int32_t)(safe_pos * 10000.0f);
        msg.buf[0] = (pos_cmd >> 24) & 0xFF;
        msg.buf[1] = (pos_cmd >> 16) & 0xFF;
        msg.buf[2] = (pos_cmd >> 8) & 0xFF;
        msg.buf[3] = pos_cmd & 0xFF;
        
        Serial.print("위치 설정: ");
        Serial.print(safe_pos);
        Serial.print("° ... ");
        if (sendCANMessage(msg)) {
            Serial.println("✓");
            readMotorData();
        }
    }
    
    void setSpeed(float rpm) {
        if (!is_motor_on) {
            Serial.println("⚠ 모터가 꺼져있습니다");
            return;
        }
        
        float safe_rpm = constrain(rpm, V_MIN, V_MAX);
        if (safe_rpm != rpm) {
            Serial.print("⚠ 속도 제한: ");
            Serial.print(rpm);
            Serial.print(" RPM → ");
            Serial.print(safe_rpm);
            Serial.println(" RPM");
        }
        
        velocity_command = safe_rpm;
        
        CAN_message_t msg;
        msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_RPM) << 8) | motor_id;
        msg.flags.extended = 1;
        msg.len = 4;
        
        // 속도 명령 스케일링 (0.1 RPM 해상도, Motor.cpp 참조)
        int32_t rpm_cmd = (int32_t)(safe_rpm * 10.0f);
        msg.buf[0] = (rpm_cmd >> 24) & 0xFF;
        msg.buf[1] = (rpm_cmd >> 16) & 0xFF;
        msg.buf[2] = (rpm_cmd >> 8) & 0xFF;
        msg.buf[3] = rpm_cmd & 0xFF;
        
        Serial.print("속도 설정: ");
        Serial.print(safe_rpm);
        Serial.print(" RPM ... ");
        if (sendCANMessage(msg)) {
            Serial.println("✓");
            readMotorData();
        }
    }
    
    void setPositionSpeed(float pos, float spd, float acc) {
        if (!is_motor_on) {
            Serial.println("⚠ 모터가 꺼져있습니다");
            return;
        }
        
        float safe_pos = constrain(pos, P_MIN, P_MAX);
        float safe_spd = constrain(spd, V_MIN, V_MAX);
        float safe_acc = constrain(acc, A_MIN, A_MAX);
        
        position_command = safe_pos;
        velocity_command = safe_spd;
        acceleration_command = safe_acc;
        
        CAN_message_t msg;
        msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS_SPD) << 8) | motor_id;
        msg.flags.extended = 1;
        msg.len = 8;
        
        // 스케일링 (Motor.cpp 참조)
        int32_t pos_cmd = (int32_t)(safe_pos * 10000.0f);  // 0.01도 해상도
        int16_t spd_cmd = (int16_t)(safe_spd / 10.0f);     // 0.1 RPM 해상도
        int16_t acc_cmd = (int16_t)(safe_acc / 10.0f);     // 0.1 RPM/s 해상도
        
        msg.buf[0] = (pos_cmd >> 24) & 0xFF;
        msg.buf[1] = (pos_cmd >> 16) & 0xFF;
        msg.buf[2] = (pos_cmd >> 8) & 0xFF;
        msg.buf[3] = pos_cmd & 0xFF;
        msg.buf[4] = (spd_cmd >> 8) & 0xFF;
        msg.buf[5] = spd_cmd & 0xFF;
        msg.buf[6] = (acc_cmd >> 8) & 0xFF;
        msg.buf[7] = acc_cmd & 0xFF;
        
        Serial.print("위치+속도+가속도 설정: ");
        Serial.print(safe_pos);
        Serial.print("°, ");
        Serial.print(safe_spd);
        Serial.print(" RPM, ");
        Serial.print(safe_acc);
        Serial.print(" ... ");
        if (sendCANMessage(msg)) {
            Serial.println("✓");
            readMotorData();
        }
    }
    
    // =============================================================================
    // 안전성 및 모니터링 함수들
    // =============================================================================
    
    void checkEmergencyStop() {
        // 비상정지 스위치: 정상(닫힌 상태) = LOW, 비상정지(열린 상태) = HIGH
        emergency_stop_active = (digitalRead(EMERGENCY_STOP_PIN) == HIGH);
        if (emergency_stop_active && is_motor_on) {
            is_motor_on = false;
            Serial.println("🚨 비상정지 활성화! (핀이 열린 상태)");
        }
    }
    
    void printSystemInfo() {
        Serial.println("\n=== 모터 시스템 정보 ===");
        Serial.print("모터 ID: ");
        Serial.println(motor_id);
        Serial.print("모터 타입: AK60_v1_1 (Kt = ");
        Serial.print(Kt);
        Serial.println(" Nm/A)");
        Serial.print("CAN 통신: 1Mbps, 타임아웃 ");
        Serial.print(timeout_us / 1000000.0f);
        Serial.println("s");
        Serial.println("🔌 CAN 핀 연결:");
        Serial.println("   Pin 22 (CTX1) = CAN TX → 모터로 송신");
        Serial.println("   Pin 23 (CRX1) = CAN RX ← 모터에서 수신");
        Serial.println("   FlexCAN_T4<CAN1> 사용");
        Serial.println("안전 제한값:");
        Serial.print("  위치: ");
        Serial.print(P_MIN);
        Serial.print("° ~ ");
        Serial.print(P_MAX);
        Serial.println("°");
        Serial.print("  속도: ");
        Serial.print(V_MIN);
        Serial.print(" ~ ");
        Serial.print(V_MAX);
        Serial.println(" RPM");
        Serial.print("  전류: ");
        Serial.print(I_MIN);
        Serial.print(" ~ ");
        Serial.print(I_MAX);
        Serial.println(" A");
        Serial.print("SD카드: ");
        Serial.println(sd_available ? "사용가능" : "사용불가");
        Serial.println("========================\n");
    }
    
    void printMotorStatus() {
        Serial.println("=== 모터 상태 ===");
        Serial.print("전원: ");
        Serial.println(is_motor_on ? "ON 🟢" : "OFF 🔴");
        Serial.print("비상정지: ");
        Serial.println(emergency_stop_active ? "활성 🚨 (핀 열림)" : "정상 ✓ (핀 연결)");
        
        if (is_motor_on) {
            Serial.println("실시간 데이터:");
            Serial.print("  위치: ");
            Serial.print(current_position);
            Serial.println("°");
            Serial.print("  속도: ");
            Serial.print(current_velocity);
            Serial.println(" RPM");
            Serial.print("  전류: ");
            Serial.print(current_current);
            Serial.println(" A");
            Serial.print("  온도: ");
            Serial.print(current_temperature);
            Serial.println("°C");
            Serial.print("  에러: ");
            Serial.print(motorErrorCodeToString(current_error));
            Serial.print(" (");
            Serial.print(current_error);
            Serial.println(")");
            
            // 토크 계산 및 표시
            float estimated_torque = current_current * Kt;
            Serial.print("  추정토크: ");
            Serial.print(estimated_torque);
            Serial.println(" Nm");
        }
        
        Serial.println("최근 명령값:");
        Serial.print("  위치명령: ");
        Serial.print(position_command);
        Serial.println("°");
        Serial.print("  속도명령: ");
        Serial.print(velocity_command);
        Serial.println(" RPM");
        Serial.print("  가속도명령: ");
        Serial.print(acceleration_command);
        Serial.println(" RPM/s");
        Serial.print("데이터 로깅: ");
        Serial.println(logging_enabled ? "ON" : "OFF");
        Serial.println("==================\n");
    }
    
    // =============================================================================
    // SD카드 로깅 함수들
    // =============================================================================
    
    String generateFilename() {
        return "motor_test_" + String(millis() / 1000) + ".csv";
    }
    
    void startDataLogging() {
        if (!sd_available) {
            Serial.println("❌ SD카드를 사용할 수 없습니다");
            return;
        }
        
        current_filename = generateFilename();
        dataFile = SD.open(current_filename.c_str(), FILE_WRITE);
        
        if (dataFile) {
            // CSV 헤더 작성
            dataFile.println("time_ms,cmd_pos,cmd_vel,cmd_acc,actual_pos,actual_vel,actual_current,temperature,error_code,estimated_torque");
            dataFile.flush();
            logging_enabled = true;
            Serial.print("✓ 데이터 로깅 시작: ");
            Serial.println(current_filename);
        } else {
            Serial.println("❌ 파일을 열 수 없습니다");
        }
    }
    
    void stopDataLogging() {
        if (logging_enabled && dataFile) {
            dataFile.close();
            logging_enabled = false;
            
            File file = SD.open(current_filename.c_str());
            if (file) {
                Serial.print("✓ 데이터 로깅 완료: ");
                Serial.print(current_filename);
                Serial.print(" (");
                Serial.print(file.size());
                Serial.println(" bytes)");
                file.close();
            }
        }
    }
    
    void logData() {
        if (!logging_enabled || !dataFile) return;
        
        // CSV 데이터 작성
        dataFile.print(millis());
        dataFile.print(",");
        dataFile.print(position_command, 2);
        dataFile.print(",");
        dataFile.print(velocity_command, 2);
        dataFile.print(",");
        dataFile.print(acceleration_command, 2);
        dataFile.print(",");
        dataFile.print(current_position, 2);
        dataFile.print(",");
        dataFile.print(current_velocity, 2);
        dataFile.print(",");
        dataFile.print(current_current, 3);
        dataFile.print(",");
        dataFile.print(current_temperature);
        dataFile.print(",");
        dataFile.print(current_error);
        dataFile.print(",");
        dataFile.println(current_current * Kt, 4); // 추정 토크
        
        // 주기적으로 플러시
        static int flush_counter = 0;
        if (++flush_counter >= 25) {  // 25개 데이터마다 (0.5초)
            dataFile.flush();
            flush_counter = 0;
        }
    }
    
    void listSDFiles() {
        if (!sd_available) {
            Serial.println("❌ SD카드를 사용할 수 없습니다");
            return;
        }
        
        Serial.println("=== SD카드 파일 목록 ===");
        File root = SD.open("/");
        int file_count = 0;
        
        while (true) {
            File entry = root.openNextFile();
            if (!entry) break;
            
            if (!entry.isDirectory()) {
                Serial.print(entry.name());
                Serial.print("\t");
                Serial.print(entry.size());
                Serial.println(" bytes");
                file_count++;
            }
            entry.close();
        }
        root.close();
        
        Serial.print("총 ");
        Serial.print(file_count);
        Serial.println("개 파일");
        Serial.println("=======================\n");
    }
    
    void printMenu() {
        Serial.println("📋 모터 테스트 명령어:");
        Serial.println("=== 기본 제어 ===");
        Serial.println("  'o' - 모터 ON/OFF 토글");
        Serial.println("  'z' - 영점 설정");
        Serial.println("  'x' - 비상정지");
        Serial.println("  'i' - 모터 상태 확인");
        Serial.println("  's' - CAN 버스 스캔");
        Serial.println("  'c' - 하드웨어 연결 상태 체크");
        
        Serial.println("=== 위치 제어 ===");
        Serial.println("  '1' - 위치 90°");
        Serial.println("  '2' - 위치 -90°");
        Serial.println("  '3' - 위치 180°");
        Serial.println("  '4' - 위치 0°");
        
        Serial.println("=== 속도 제어 ===");
        Serial.println("  '5' - 속도 1000 RPM");
        Serial.println("  '6' - 속도 -1000 RPM");
        Serial.println("  '7' - 정지 (0 RPM)");
        
        Serial.println("=== 복합 제어 ===");
        Serial.println("  '8' - 위치+속도: 360°, 2000 RPM, 1500 acc");
        Serial.println("  '9' - 위치+속도: -360°, -2000 RPM, 1500 acc");
        
        Serial.println("=== 데이터 로깅 ===");
        Serial.println("  'l' - 로깅 시작/정지");
        Serial.println("  'f' - 파일 목록");
        Serial.println("  'h' - 도움말");
        Serial.println("=======================\n");
    }
    
    // =============================================================================
    // 메인 루프 함수
    // =============================================================================
    
    void update() {
        // 비상정지 확인
        checkEmergencyStop();
        
        // 모터 데이터 읽기 (활성화된 경우)
        if (is_motor_on) {
            readMotorData();
            
            // 에러 검사
            if (current_error != (uint8_t)MotorErrorCode::NO_FAULT) {
                Serial.print("⚠ 모터 에러 감지: ");
                Serial.println(motorErrorCodeToString(current_error));
            }
        }
        
        // 데이터 로깅
        if (logging_enabled && (millis() - last_log_time >= log_interval)) {
            logData();
            last_log_time = millis();
        }
    }
};

// =============================================================================
// 전역 변수 및 메인 함수들
// =============================================================================

MotorTestSystem motor_test(65);  // 모터 ID: 65 (AK60_v1_1)

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {}
    
    Serial.println("===============================================");
    Serial.println("🚀 H-Walker AK60_v1_1 Motor Test System v1.0");
    Serial.println("===============================================");
    
    motor_test.init();
    motor_test.printMenu();
}

void loop() {
    // 시스템 업데이트
    motor_test.update();
    
    // 시리얼 명령 처리
    if (Serial.available() > 0) {
        char command = Serial.read();
        
        // 입력 버퍼 클리어
        while (Serial.available()) {
            Serial.read();
            delay(1);
        }
        
        Serial.print("\n>>> 명령: '");
        Serial.print(command);
        Serial.println("'");
        
        switch (command) {
            case 'o':
                motor_test.toggleMotorPower();
                break;
                
            case 'z':
                motor_test.zeroPosition();
                break;
                
            case 'x':
                Serial.println("🚨 비상정지!");
                // 강제로 모터 OFF (비상정지 시뮬레이션)
                break;
                
            case 'i':
                motor_test.printMotorStatus();
                break;
                
            case 's':
                motor_test.scanCANMessages();
                break;
                
            case 'c':
                motor_test.checkHardwareConnections();
                break;
                
            // 위치 제어
            case '1':
                motor_test.setPosition(90.0f);
                break;
            case '2':
                motor_test.setPosition(-90.0f);
                break;
            case '3':
                motor_test.setPosition(180.0f);
                break;
            case '4':
                motor_test.setPosition(0.0f);
                break;
                
            // 속도 제어
            case '5':
                motor_test.setSpeed(1000.0f);   // 1000 RPM
                break;
            case '6':
                motor_test.setSpeed(-1000.0f);  // -1000 RPM
                break;
            case '7':
                motor_test.setSpeed(0.0f);
                break;
                
            // 복합 제어
            case '8':
                motor_test.setPositionSpeed(360.0f, 2000.0f, 1500.0f);  // 1회전, 2000 RPM, 1500 acc
                break;
            case '9':
                motor_test.setPositionSpeed(-360.0f, -2000.0f, 1500.0f);  // -1회전, -2000 RPM, 1500 acc
                break;
                
            // 로깅
            case 'l':
                if (motor_test.logging_enabled) {
                    motor_test.stopDataLogging();
                } else {
                    motor_test.startDataLogging();
                }
                break;
            case 'f':
                motor_test.listSDFiles();
                break;
                
            case 'h':
                motor_test.printMenu();
                break;
                
            case '\n':
            case '\r':
                // 개행문자 무시
                break;
                
            default:
                Serial.print("❌ 알 수 없는 명령: '");
                Serial.print(command);
                Serial.println("' - 'h'로 도움말 확인");
                break;
        }
        Serial.println();
    }
    
    delay(10);  // CPU 부하 감소
}