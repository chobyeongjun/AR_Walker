/*
 * CAN 수신 테스트 전용 프로그램
 * 
 * 목적: CAN 버스에서 메시지가 수신되는지만 확인
 * 기능: 모든 CAN 메시지를 실시간으로 표시
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
// CAN1 = Pin 22 (CTX1 송신), Pin 23 (CRX1 수신)
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#endif

// 통계 변수
unsigned long message_count = 0;
unsigned long last_stats_time = 0;
bool any_message_received = false;

// ID별 수신 통계 (0~100)
unsigned long id_count[101] = {0};  // ID 0~100까지
bool id_detected[101] = {false};    // 각 ID 감지 여부
unsigned long last_summary_time = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {}
    
    Serial.println("=======================================");
    Serial.println("🔍 CAN 수신 테스트 프로그램");
    Serial.println("=======================================");
    Serial.println("📌 목적: CAN 버스 메시지 수신 확인");
    Serial.println("📌 핀: Pin 22 (CTX1), Pin 23 (CRX1)");
    Serial.println("📌 CAN 컨트롤러: FlexCAN_T4<CAN1>");
    Serial.println("=======================================");
    
    // CAN 초기화
    Serial.print("🔧 CAN 초기화 중... ");
    Can0.begin();
    Can0.setBaudRate(1000000); // 1Mbps
    Serial.println("완료!");
    
    Serial.println("");
    Serial.println("📡 CAN 메시지 수신 대기 중...");
    Serial.println("수신되는 모든 메시지를 실시간으로 표시합니다.");
    Serial.println("🔍 ID 0~100 범위의 메시지를 자동으로 추적합니다.");
    Serial.println("Ctrl+C로 종료하세요.");
    Serial.println("---------------------------------------");
    
    last_stats_time = millis();
}

void loop() {
    CAN_message_t msg;
    
    // CAN 메시지 수신 확인
    if (Can0.read(msg)) {
        message_count++;
        any_message_received = true;
        
        // 메시지 정보 표시
        Serial.print("📥 #");
        Serial.print(message_count);
        Serial.print(": ID=0x");
        Serial.print(msg.id, HEX);
        Serial.print(" (");
        Serial.print(msg.id);
        Serial.print(")");
        
        // Extended/Standard 프레임 표시
        Serial.print(msg.flags.extended ? " [EXT]" : " [STD]");
        
        Serial.print(", Len=");
        Serial.print(msg.len);
        Serial.print(", Data=[");
        
        // 데이터 바이트 표시
        for (int i = 0; i < msg.len && i < 8; i++) {
            if (msg.buf[i] < 16) Serial.print("0");
            Serial.print(msg.buf[i], HEX);
            if (i < msg.len - 1) Serial.print(" ");
        }
        Serial.print("], Time=");
        Serial.println(millis());
        
        // ID 0~100 범위 추적
        uint32_t full_id = msg.id;
        uint8_t low_id = msg.id & 0xFF;  // 하위 8비트
        
        if (low_id <= 100) {
            id_count[low_id]++;
            if (!id_detected[low_id]) {
                id_detected[low_id] = true;
                Serial.print("  🎯 새로운 ID 감지: ");
                Serial.print(low_id);
                Serial.println(" (0~100 범위)");
            }
            
            // 모터 데이터로 보이는 메시지 파싱 (8바이트 길이)
            if (msg.len == 8) {
                Serial.print("  📊 ID ");
                Serial.print(low_id);
                Serial.print(" 데이터 파싱: ");
                
                int16_t pos = (msg.buf[0] << 8) | msg.buf[1];
                int16_t vel = (msg.buf[2] << 8) | msg.buf[3];
                int16_t cur = (msg.buf[4] << 8) | msg.buf[5];
                int8_t temp = msg.buf[6];
                uint8_t err = msg.buf[7];
                
                Serial.print("위치=");
                Serial.print(pos * 0.1f);
                Serial.print("°, 속도=");
                Serial.print(vel * 10.0f);
                Serial.print("RPM, 전류=");
                Serial.print(cur * 0.01f);
                Serial.print("A, 온도=");
                Serial.print(temp);
                Serial.print("°C, 에러=");
                Serial.println(err);
            }
        }
        
        // Extended 프레임의 경우 상위 비트도 확인
        if (msg.flags.extended && full_id > 0xFF) {
            uint8_t upper_id = (full_id >> 8) & 0xFF;
            if (upper_id <= 100 && upper_id != low_id) {
                Serial.print("  🔍 Extended ID 상위: ");
                Serial.print(upper_id);
                Serial.print(" (전체 ID: 0x");
                Serial.print(full_id, HEX);
                Serial.println(")");
            }
        }
    }
    
    // 1초마다 통계 표시
    if (millis() - last_stats_time >= 1000) {
        if (!any_message_received) {
            Serial.print("⏰ ");
            Serial.print((millis() / 1000));
            Serial.println("초 경과 - 메시지 없음");
            
            // 10초마다 하드웨어 체크 안내
            if ((millis() / 1000) % 10 == 0) {
                Serial.println("🔧 하드웨어 확인사항:");
                Serial.println("   1. CAN 트랜시버 연결 확인");
                Serial.println("   2. 모터 24V 전원 확인");
                Serial.println("   3. CAN H/L 케이블 확인");
            }
        } else {
            Serial.print("📊 총 ");
            Serial.print(message_count);
            Serial.println("개 메시지 수신됨");
        }
        last_stats_time = millis();
    }
    
    // 10초마다 ID 통계 요약 표시
    if (any_message_received && millis() - last_summary_time >= 10000) {
        Serial.println("");
        Serial.println("📈 ID별 수신 통계 (0~100 범위):");
        Serial.println("ID   | 메시지 수 | 상태");
        Serial.println("-----|----------|------");
        
        bool found_active_ids = false;
        for (int i = 0; i <= 100; i++) {
            if (id_detected[i]) {
                Serial.print(i < 10 ? " " : "");
                Serial.print(i);
                Serial.print("   | ");
                Serial.print(id_count[i] < 10 ? "       " : (id_count[i] < 100 ? "      " : "     "));
                Serial.print(id_count[i]);
                Serial.println(" | 활성");
                found_active_ids = true;
            }
        }
        
        if (!found_active_ids) {
            Serial.println("(0~100 범위에서 활성 ID 없음)");
        }
        
        Serial.println("---------------------------");
        last_summary_time = millis();
    }
    
    delay(1);  // CPU 부하 감소
}