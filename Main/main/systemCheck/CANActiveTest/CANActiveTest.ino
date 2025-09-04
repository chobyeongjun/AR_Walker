/*
 * CAN 능동 스캐너 - ID 0~100 전체 스캔
 * 
 * 목적: 각 ID로 메시지를 보내서 응답하는 장치 찾기
 * 방식: ID 0부터 100까지 순서대로 명령을 보내고 응답 확인
 * 
 * 하드웨어:
 * - Teensy 4.1
 * - Pin 22 (CTX1), Pin 23 (CRX1)
 * - CAN 트랜시버 (MCP2515, MCP2562 등)
 */

#include "Arduino.h"
#include "FlexCAN_T4.h"

// Teensy 4.1 CAN 설정
#if defined(ARDUINO_TEENSY41)
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#endif

// 스캔 상태
uint8_t current_scan_id = 0;
unsigned long last_scan_time = 0;
const unsigned long scan_interval = 500;  // 각 ID마다 500ms 대기
const unsigned long response_timeout = 300; // 응답 대기 300ms

// 응답 통계
bool id_responded[101] = {false};  // ID 0~100 응답 여부
unsigned long response_count[101] = {0};  // 각 ID 응답 횟수
int total_responding_ids = 0;

// 스캔 모드
enum ScanMode {
    SCAN_IDLE,
    SCAN_SENDING,
    SCAN_WAITING,
    SCAN_COMPLETE
};

ScanMode scan_mode = SCAN_IDLE;
unsigned long scan_start_time = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {}
    
    Serial.println("=============================================");
    Serial.println("🔍 CAN 능동 스캐너 v1.0");
    Serial.println("=============================================");
    Serial.println("📌 기능: ID 0~100 순차 스캔 및 응답 확인");
    Serial.println("📌 핀: Pin 22 (CTX1), Pin 23 (CRX1)");
    Serial.println("📌 방식: 각 ID로 상태 요청 메시지 전송");
    Serial.println("=============================================");
    
    // CAN 초기화
    Serial.print("🔧 CAN 초기화 중... ");
    Can0.begin();
    Can0.setBaudRate(1000000); // 1Mbps
    Serial.println("완료!");
    
    Serial.println("");
    Serial.println("📋 명령어:");
    Serial.println("  's' - 스캔 시작");
    Serial.println("  'r' - 결과 보기");
    Serial.println("  'c' - 초기화");
    Serial.println("---------------------------------------------");
}

// 특정 ID로 상태 요청 메시지 전송
void sendStatusRequest(uint8_t motor_id) {
    CAN_message_t msg;
    
    // 모터 상태 요청 메시지 (일반적인 형태)
    msg.id = (0x04 << 8) | motor_id;  // SET_POS 명령으로 응답 유도
    msg.flags.extended = 1;
    msg.len = 4;
    msg.buf[0] = 0x00;  // 위치 0
    msg.buf[1] = 0x00;
    msg.buf[2] = 0x00;
    msg.buf[3] = 0x00;
    
    Serial.print("📤 ID ");
    Serial.print(motor_id);
    Serial.print(" 스캔 중... ");
    
    if (Can0.write(msg)) {
        Serial.print("전송 OK");
    } else {
        Serial.print("전송 실패");
    }
}

// 응답 메시지 확인
bool checkForResponse(uint8_t expected_id, unsigned long timeout_ms) {
    CAN_message_t msg;
    unsigned long start_time = millis();
    bool found_response = false;
    
    while ((millis() - start_time) < timeout_ms) {
        if (Can0.read(msg)) {
            uint8_t msg_id = msg.id & 0xFF;
            
            // 해당 ID에서 온 응답인지 확인
            if (msg_id == expected_id) {
                Serial.print(" → ✅ 응답 수신!");
                
                if (msg.len == 8) {
                    // 모터 데이터 파싱
                    int16_t pos = (msg.buf[0] << 8) | msg.buf[1];
                    int16_t vel = (msg.buf[2] << 8) | msg.buf[3];
                    int16_t cur = (msg.buf[4] << 8) | msg.buf[5];
                    int8_t temp = msg.buf[6];
                    uint8_t err = msg.buf[7];
                    
                    Serial.print(" [위치:");
                    Serial.print(pos * 0.1f);
                    Serial.print("° 속도:");
                    Serial.print(vel * 10.0f);
                    Serial.print("RPM 전류:");
                    Serial.print(cur * 0.01f);
                    Serial.print("A 온도:");
                    Serial.print(temp);
                    Serial.print("°C 에러:");
                    Serial.print(err);
                    Serial.print("]");
                }
                
                Serial.println("");
                
                // 통계 업데이트
                if (!id_responded[expected_id]) {
                    id_responded[expected_id] = true;
                    total_responding_ids++;
                }
                response_count[expected_id]++;
                
                found_response = true;
                break;
            } else {
                // 다른 ID에서 온 메시지
                Serial.print("(ID ");
                Serial.print(msg_id);
                Serial.print(" 수신) ");
            }
        }
        delay(1);
    }
    
    if (!found_response) {
        Serial.println(" → ❌ 응답 없음");
    }
    
    return found_response;
}

// 전체 스캔 실행
void performFullScan() {
    Serial.println("\n🚀 ID 0~100 전체 스캔 시작!");
    Serial.println("각 ID마다 상태 요청을 보내고 응답을 확인합니다...\n");
    
    // 초기화
    for (int i = 0; i <= 100; i++) {
        id_responded[i] = false;
        response_count[i] = 0;
    }
    total_responding_ids = 0;
    
    unsigned long scan_start = millis();
    
    // ID 0부터 100까지 순차 스캔
    for (uint8_t id = 0; id <= 100; id++) {
        sendStatusRequest(id);
        checkForResponse(id, response_timeout);
        
        delay(100); // ID간 간격
        
        // 진행률 표시 (10개씩)
        if ((id + 1) % 20 == 0) {
            Serial.print("📊 진행률: ");
            Serial.print(((id + 1) * 100) / 101);
            Serial.print("% (");
            Serial.print(id + 1);
            Serial.println("/101)");
        }
    }
    
    unsigned long scan_duration = millis() - scan_start;
    
    Serial.println("\n✅ 스캔 완료!");
    Serial.print("⏱️ 소요시간: ");
    Serial.print(scan_duration / 1000.0f);
    Serial.println("초");
    
    // 결과 요약
    showScanResults();
}

// 스캔 결과 표시
void showScanResults() {
    Serial.println("\n📈 스캔 결과 요약:");
    Serial.println("===========================================");
    Serial.print("🎯 응답한 ID 수: ");
    Serial.print(total_responding_ids);
    Serial.println("/101");
    
    if (total_responding_ids > 0) {
        Serial.println("\n📋 응답한 ID 목록:");
        Serial.println("ID   | 응답 수 | 상태");
        Serial.println("-----|---------|--------");
        
        for (int i = 0; i <= 100; i++) {
            if (id_responded[i]) {
                Serial.print(i < 10 ? " " : "");
                Serial.print(i);
                Serial.print("   | ");
                Serial.print(response_count[i] < 10 ? "   " : (response_count[i] < 100 ? "  " : " "));
                Serial.print(response_count[i]);
                Serial.println("     | 🟢 활성");
            }
        }
    } else {
        Serial.println("\n❌ 응답한 ID가 없습니다.");
        Serial.println("🔧 확인사항:");
        Serial.println("   1. 모터 전원 24V 확인");
        Serial.println("   2. CAN 트랜시버 연결 확인");
        Serial.println("   3. CAN H/L 케이블 확인");
        Serial.println("   4. 종단저항 120Ω 확인");
    }
    Serial.println("===========================================\n");
}

void loop() {
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
            case 's':
                performFullScan();
                break;
                
            case 'r':
                showScanResults();
                break;
                
            case 'c':
                // 초기화
                for (int i = 0; i <= 100; i++) {
                    id_responded[i] = false;
                    response_count[i] = 0;
                }
                total_responding_ids = 0;
                Serial.println("✅ 스캔 결과 초기화 완료");
                break;
                
            case '\n':
            case '\r':
                break;
                
            default:
                Serial.print("❌ 알 수 없는 명령: '");
                Serial.print(command);
                Serial.println("'");
                Serial.println("사용 가능한 명령: 's' (스캔), 'r' (결과), 'c' (초기화)");
                break;
        }
    }
    
    delay(10);
}