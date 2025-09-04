/*
 * MediWalker Unified Admittance Test System
 * 
 * 기능:
 * - m,c,s 시리얼 파라미터 제어 (예: m1.5,c30,s100)
 * - Reference Force 크기 조절 (시리얼 입력)
 * - Half-sine 또는 Step 패턴 선택 (컴파일 시 설정)
 * - 제어 주파수 선택 (100/200/500Hz - 컴파일 시 설정)
 * - SD카드 정확한 데이터 로깅: Timer, Actual Force, Ref Force, M/D/K, Error%
 * 
 * 하드웨어:
 * - Teensy 4.1
 * - AK60-6 모터 (CAN ID: 65)
 * - 로드셀 (A16 핀)
 */

#include "Arduino.h"
#include "FlexCAN_T4.h"
#include "SD.h"
#include <math.h>

// =============================================================================
// 컴파일 시 설정 (사용자가 선택)
// =============================================================================

#define FORCE_PATTERN_HALF_SINE  // 또는 FORCE_PATTERN_STEP 으로 변경
#define CONTROL_FREQ_HZ 500      // 제어 주파수 (100, 200, 500Hz 중 선택)

// =============================================================================
// 하드웨어 설정
// =============================================================================

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#define LOADCELL_PIN A16   // Left Loadcell (Left Knee)
#define MOTOR_ID 65        // Left Knee Motor

// 로드셀 캐리브레이션 상수 (LoadcellTest에서 가져옴)
namespace loadcell_calib {
    const float AI_CNT_TO_V = 3.3f / 4096.0f;  // ADC 카운트를 전압으로 변환
    
    // Left Knee 캐리브레이션 값
    const float left_knee_bias = -682.8357f;
    const float left_knee_sensitive = 560.7490f;
    
    // Right Knee 캐리브레이션 값
    const float right_knee_bias = -654.4153f;
    const float right_knee_sensitive = 541.1179f;
}

// =============================================================================
// CAN 통신 정의
// =============================================================================

enum class CAN_PACKET_ID : uint8_t {
    SET_POS_SPD = 6
};

// =============================================================================
// 시스템 상태
// =============================================================================

struct {
    // Admittance 파라미터
    float mass = 1.0f;
    float damping = 0.0f;
    float stiffness = 0.0f;
    
    // Force 제어
    float reference_force = 0.0f;
    float actual_force = 0.0f;
    float force_amplitude = 10.0f;  // 시리얼로 조절 가능
    
    // 위치 상태
    float reference_position = 0.0f;
    float reference_velocity = 0.0f;
    
    // 시간 추적
    unsigned long start_time = 0;
    float dt = 1.0f / CONTROL_FREQ_HZ;  // 제어 주기 자동 계산
    
    // 시스템 제어
    bool system_running = false;
    bool test_active = false;
    
} state;

// SD카드 로깅
File dataFile;
bool logging_active = false;
String filename = "";

// =============================================================================
// 초기화
// =============================================================================

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {}
    
    Serial.println("====================================");
    Serial.println("🎯 MediWalker Unified Admittance Test");
    Serial.println("====================================");
    
    // 하드웨어 초기화
    pinMode(LOADCELL_PIN, INPUT);
    analogReadResolution(12);
    
    // CAN 초기화
    Can0.begin();
    Can0.setBaudRate(1000000);
    
    // SD카드 초기화
    if (SD.begin(BUILTIN_SDCARD)) {
        Serial.println("✓ SD카드 준비됨");
    } else {
        Serial.println("❌ SD카드 초기화 실패");
    }
    
    // 제어 주파수 출력
    Serial.print("⚙️ 제어 주파수: ");
    Serial.print(CONTROL_FREQ_HZ);
    Serial.print("Hz (");
    Serial.print(1000.0f / CONTROL_FREQ_HZ, 1);
    Serial.println("ms)");
    
    // 패턴 정보 출력
    #ifdef FORCE_PATTERN_HALF_SINE
    Serial.println("📊 Force Pattern: Half-Sine");
    #else
    Serial.println("📊 Force Pattern: Step");
    #endif
    
    // 로드셀 캘리브레이션 값 출력
    Serial.println("🔧 로드셀 캘리브레이션 (Left Knee):");
    Serial.print("   Bias: ");
    Serial.print(loadcell_calib::left_knee_bias, 4);
    Serial.print(", Sensitive: ");
    Serial.println(loadcell_calib::left_knee_sensitive, 4);
    
    printCommands();
    state.start_time = millis();
}

// =============================================================================
// Reference Force 생성
// =============================================================================

void updateReferenceForce() {
    float time_s = (millis() - state.start_time) / 1000.0f;
    
    #ifdef FORCE_PATTERN_HALF_SINE
    // Half-sine 패턴 (주기: 4초)
    float period = 4.0f;
    float cycle_time = fmod(time_s, period);
    
    if (cycle_time < period / 2.0f) {
        // 2초 동안 half-sine
        state.reference_force = state.force_amplitude * sin(PI * cycle_time / (period / 2.0f));
    } else {
        // 2초 동안 0
        state.reference_force = 0.0f;
    }
    
    #else
    // Step 패턴
    float step_time = fmod(time_s, 8.0f);
    if (step_time < 2.0f) {
        state.reference_force = 0.0f;
    } else if (step_time < 4.0f) {
        state.reference_force = state.force_amplitude;
    } else if (step_time < 6.0f) {
        state.reference_force = 0.0f;
    } else {
        state.reference_force = -state.force_amplitude;
    }
    #endif
}

// =============================================================================
// Admittance Control 계산
// =============================================================================

void calculateAdmittanceControl() {
    // 로드셀 읽기 (LoadcellTest에서 설정한 캐리브레이션 사용)
    int raw_reading = analogRead(LOADCELL_PIN);
    float voltage = (float)raw_reading * loadcell_calib::AI_CNT_TO_V;
    
    // Left Knee 캐리브레이션 적용 (right_knee로 바꿀 때 A6로 바꾸면)
    state.actual_force = (voltage * loadcell_calib::left_knee_sensitive + loadcell_calib::left_knee_bias);
    
    // Force error 계산
    float force_error = state.reference_force - state.actual_force;
    
    // Admittance equation: M*a + D*v + K*x = F_error
    float acceleration = (force_error - state.damping * state.reference_velocity 
                         - state.stiffness * state.reference_position) / state.mass;
    
    // 적분
    state.reference_velocity += acceleration * state.dt;
    state.reference_position += state.reference_velocity * state.dt;
    
    // 안전 제한
    state.reference_velocity = constrain(state.reference_velocity, -8000.0f, 8000.0f);
    state.reference_position = constrain(state.reference_position, -1800.0f, 1800.0f);
}

// =============================================================================
// 모터 제어
// =============================================================================

void sendMotorCommand() {
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS_SPD) << 8) | MOTOR_ID;
    msg.flags.extended = 1;
    msg.len = 8;
    
    // 위치와 속도 명령 (LoadcellTest에서 설정한 방식 사용)
    int32_t pos_cmd = (int32_t)(state.reference_position * 10000.0f);
    int16_t vel_cmd = 8000/10;
    int16_t acc_cmd = 1000/10; // 고정 가속도
    
    msg.buf[0] = (pos_cmd >> 24) & 0xFF;
    msg.buf[1] = (pos_cmd >> 16) & 0xFF;
    msg.buf[2] = (pos_cmd >> 8) & 0xFF;
    msg.buf[3] = pos_cmd & 0xFF;
    msg.buf[4] = (vel_cmd >> 8) & 0xFF;
    msg.buf[5] = vel_cmd & 0xFF;
    msg.buf[6] = (acc_cmd >> 8) & 0xFF;
    msg.buf[7] = acc_cmd & 0xFF;
    
    Can0.write(msg);
}

// =============================================================================
// SD카드 로깅
// =============================================================================

void startLogging() {
    filename = "admit_" + String(millis() / 1000) + ".csv";
    dataFile = SD.open(filename.c_str(), FILE_WRITE);
    
    if (dataFile) {
        // 사용자 요구 헤더: Timer, Actual Force, Ref Force, M/D/K, Error%
        dataFile.println("timer_ms,actual_force,ref_force,mass,damping,stiffness,error_percent");
        dataFile.flush();
        logging_active = true;
        Serial.print("✓ 로깅 시작: ");
        Serial.println(filename);
    } else {
        Serial.println("❌ 파일 생성 실패");
    }
}

void stopLogging() {
    if (logging_active && dataFile) {
        dataFile.close();
        logging_active = false;
        Serial.print("✓ 로깅 완료: ");
        Serial.println(filename);
    }
}

void logData() {
    if (!logging_active || !dataFile) return;
    
    // Error percentage 계산
    float error_percent = 0.0f;
    if (abs(state.reference_force) > 0.01f) {
        error_percent = (abs(state.reference_force - state.actual_force) / abs(state.reference_force)) * 100.0f;
    }
    
    // 사용자 요구 형식으로 로깅
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(state.actual_force, 4);
    dataFile.print(",");
    dataFile.print(state.reference_force, 4);
    dataFile.print(",");
    dataFile.print(state.mass, 4);
    dataFile.print(",");
    dataFile.print(state.damping, 4);
    dataFile.print(",");
    dataFile.print(state.stiffness, 4);
    dataFile.print(",");
    dataFile.println(error_percent, 2);
    
    // 주기적 플러시
    static int flush_counter = 0;
    if (++flush_counter >= 50) {
        dataFile.flush();
        flush_counter = 0;
    }
}

// =============================================================================
// 시리얼 명령 처리
// =============================================================================

void parseParameters(String input) {
    // m1.5,c30,s100 형식 파싱
    input.replace(" ", ""); // 공백 제거
    input.toLowerCase();
    
    int start = 0;
    int comma_pos = 0;
    
    Serial.print("파라미터 파싱: ");
    Serial.println(input);
    
    while (comma_pos != -1) {
        comma_pos = input.indexOf(',', start);
        String param;
        
        if (comma_pos != -1) {
            param = input.substring(start, comma_pos);
            start = comma_pos + 1;
        } else {
            param = input.substring(start);
        }
        
        if (param.length() >= 2) {
            char type = param.charAt(0);
            float value = param.substring(1).toFloat();
            
            switch (type) {
                case 'm':
                    state.mass = constrain(value, 0.1f, 50.0f);
                    Serial.print("Mass = ");
                    Serial.println(state.mass);
                    break;
                case 'c':
                    state.damping = constrain(value, 0.1f, 1000.0f);
                    Serial.print("Damping = ");
                    Serial.println(state.damping);
                    break;
                case 's':
                    state.stiffness = constrain(value, 0.0f, 10000.0f);
                    Serial.print("Stiffness = ");
                    Serial.println(state.stiffness);
                    break;
                default:
                    Serial.print("❌ 알 수 없는 파라미터: ");
                    Serial.println(type);
                    break;
            }
        }
    }
    
    Serial.println("✓ 파라미터 업데이트 완료");
}

void processSerialCommand() {
    if (!Serial.available()) return;
    
    String input = Serial.readString();
    input.trim();
    
    if (input.length() == 0) return;
    
    Serial.print(">>> 입력: ");
    Serial.println(input);
    
    // m,c,s 형식 확인
    if (input.indexOf(',') != -1 && (input.indexOf('m') != -1 || input.indexOf('c') != -1 || input.indexOf('s') != -1)) {
        parseParameters(input);
        return;
    }
    
    // 단일 명령어 처리
    char cmd = input.charAt(0);
    
    switch (cmd) {
        case 'o':
            state.system_running = !state.system_running;
            Serial.print("시스템: ");
            Serial.println(state.system_running ? "ON" : "OFF");
            break;
            
        case 'r':
            if (state.system_running) {
                state.test_active = !state.test_active;
                if (state.test_active) {
                    state.start_time = millis();
                    state.reference_position = 0.0f;
                    state.reference_velocity = 0.0f;
                    Serial.println("✓ 테스트 시작");
                } else {
                    Serial.println("✓ 테스트 정지");
                }
            } else {
                Serial.println("⚠ 먼저 시스템을 켜주세요 (o)");
            }
            break;
            
        case 'f':
            Serial.print("Force 크기 입력 (현재 ");
            Serial.print(state.force_amplitude);
            Serial.print("): ");
            while (Serial.available() == 0) delay(50);
            {
                float new_force = Serial.parseFloat();
                state.force_amplitude = constrain(new_force, 0.1f, 50.0f);
                Serial.print("Force = ");
                Serial.println(state.force_amplitude);
            }
            break;
            
        case 'l':
            if (logging_active) {
                stopLogging();
            } else {
                startLogging();
            }
            break;
            
        case 'i':
            printStatus();
            break;
            
        case 'h':
            printCommands();
            break;
            
        default:
            Serial.println("❌ 알 수 없는 명령. 'h'로 도움말 확인");
            break;
    }
}

// =============================================================================
// 정보 출력
// =============================================================================

void printCommands() {
    Serial.println("\n📋 명령어:");
    Serial.println("  'o' - 시스템 ON/OFF");
    Serial.println("  'r' - 테스트 준비");
    Serial.println("  'f' - Force 설정 & 생성 시작");
    Serial.println("  'l' - 로깅 시작/정지");
    Serial.println("  'i' - 상태 확인");
    Serial.println("  'h' - 도움말");
    Serial.println("  파라미터: m1.5,c30,s100 (엔터)");
    Serial.println("\n🔧 사용 순서: o → m,c,s → r → f");
    Serial.println("==========================\n");
}

void printStatus() {
    Serial.println("\n=== 시스템 상태 ===");
    Serial.print("시스템: ");
    Serial.println(state.system_running ? "ON" : "OFF");
    Serial.print("테스트: ");
    Serial.println(state.test_active ? "실행중" : "정지");
    Serial.print("로깅: ");
    Serial.println(logging_active ? "ON" : "OFF");
    
    Serial.println("\n--- 파라미터 ---");
    Serial.print("Mass: ");
    Serial.println(state.mass);
    Serial.print("Damping: ");
    Serial.println(state.damping);
    Serial.print("Stiffness: ");
    Serial.println(state.stiffness);
    Serial.print("Force 크기: ");
    Serial.println(state.force_amplitude);
    
    if (state.test_active) {
        Serial.println("\n--- 실시간 데이터 ---");
        Serial.print("Ref Force: ");
        Serial.println(state.reference_force);
        Serial.print("Actual Force: ");
        Serial.println(state.actual_force);
        Serial.print("Ref Position: ");
        Serial.println(state.reference_position);
    }
    Serial.println("================\n");
}

// =============================================================================
// 메인 루프
// =============================================================================

void loop() {
    // 시리얼 명령 처리
    processSerialCommand();
    
    // 시스템 실행 중일 때만 제어 루프
    if (state.system_running && state.test_active) {
        static unsigned long last_control = 0;
        unsigned long now = micros();
        
        if (now - last_control >= (1000000 / CONTROL_FREQ_HZ)) { // 설정된 주파수로 제어
            // Reference force 업데이트
            updateReferenceForce();
            
            // Admittance control 계산
            calculateAdmittanceControl();
            
            // 모터 명령 전송
            sendMotorCommand();
            
            // 데이터 로깅 (100Hz) - 주파수에 따라 자동 조정
            static int log_counter = 0;
            int log_divider = CONTROL_FREQ_HZ / 100; // 100Hz 로깅을 위한 나누기
            if (log_divider < 1) log_divider = 1;    // 최소 1
            if (++log_counter >= log_divider) {
                logData();
                log_counter = 0;
            }
            
            last_control = now;
        }
    }
    
    delay(1); // 1ms delay for 100Hz main loop
}