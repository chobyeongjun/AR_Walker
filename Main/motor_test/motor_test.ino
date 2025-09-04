#include "FlexCAN_T4.h"

#if defined(ARDUINO_TEENSY36)
    FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> Can0;
#elif defined(ARDUINO_TEENSY41)
    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#endif

enum class CAN_PACKET_ID : uint8_t {
  SET_DUTY = 0,
  SET_CURRENT = 1,
  SET_CURRENT_BRAKE = 2,
  SET_RPM = 3,
  SET_POS = 4,
  SET_ORIGIN_HERE = 5,
  SET_POS_SPD = 6
};

class MotorTest {
private:
  uint8_t motor_id;
  bool is_motor_on;
  
  // 안전 제한값
  float P_MAX = 3600.0f;   // 최대 위치 (도)
  float P_MIN = -3600.0f;  // 최소 위치 (도)  
  float V_MAX = 5000.0f;   // 최대 속도 (RPM)
  float V_MIN = -5000.0f;  // 최소 속도 (RPM)
  float A_MAX = 200.0f;    // 최대 가속도
  float A_MIN = 0.0f;      // 최소 가속도

public:
  MotorTest(uint8_t id) {
    motor_id = id;
    is_motor_on = false;
  }

  void init() {
    Can0.begin();
    Can0.setBaudRate(1000000); // 1Mbps
    Serial.println("CAN Bus initialized at 1Mbps");
  }

  void toggle_power() {
    is_motor_on = !is_motor_on;
    Serial.print("Motor ");
    Serial.print(motor_id);
    Serial.print(" is now ");
    Serial.println(is_motor_on ? "ON" : "OFF");
  }

  bool send_can_message(CAN_message_t &msg) {
    if (Can0.write(msg)) {
      Serial.println("✓ CAN message sent successfully");
      return true;
    } else {
      Serial.println("✗ Failed to send CAN message");
      return false;
    }
  }

  void zero_position() {
    if (!is_motor_on) {
      Serial.println("⚠ Motor is OFF - cannot zero position");
      return;
    }
    
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_ORIGIN_HERE) << 8) | motor_id;
    msg.flags.extended = true;
    msg.len = 1;
    msg.buf[0] = 0;
    
    Serial.print("Sending zero command to motor ");
    Serial.print(motor_id);
    Serial.print("... ");
    send_can_message(msg);
  }

  void set_position(float degrees) {
    if (!is_motor_on) {
      Serial.println("⚠ Motor is OFF - cannot set position");
      return;
    }
    
    float safe_pos = constrain(degrees, P_MIN, P_MAX);
    if (safe_pos != degrees) {
      Serial.print("⚠ Position clamped from ");
      Serial.print(degrees);
      Serial.print("° to ");
      Serial.print(safe_pos);
      Serial.println("°");
    }
    
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS) << 8) | motor_id;
    msg.flags.extended = true;
    msg.len = 4;
    
    int32_t pos_scaled = (int32_t)(safe_pos * 10000.0f);
    msg.buf[0] = (pos_scaled >> 24) & 0xFF;
    msg.buf[1] = (pos_scaled >> 16) & 0xFF;
    msg.buf[2] = (pos_scaled >> 8) & 0xFF;
    msg.buf[3] = pos_scaled & 0xFF;
    
    Serial.print("Setting position to ");
    Serial.print(safe_pos);
    Serial.print("°... ");
    send_can_message(msg);
  }

  void set_speed(float rpm) {
    if (!is_motor_on) {
      Serial.println("⚠ Motor is OFF - cannot set speed");
      return;
    }
    
    float safe_rpm = constrain(rpm, V_MIN, V_MAX);
    if (safe_rpm != rpm) {
      Serial.print("⚠ Speed clamped from ");
      Serial.print(rpm);
      Serial.print(" RPM to ");
      Serial.print(safe_rpm);
      Serial.println(" RPM");
    }
    
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_RPM) << 8) | motor_id;
    msg.flags.extended = true;
    msg.len = 4;
    
    int32_t rpm_scaled = (int32_t)safe_rpm;
    msg.buf[0] = (rpm_scaled >> 24) & 0xFF;
    msg.buf[1] = (rpm_scaled >> 16) & 0xFF;
    msg.buf[2] = (rpm_scaled >> 8) & 0xFF;
    msg.buf[3] = rpm_scaled & 0xFF;
    
    Serial.print("Setting speed to ");
    Serial.print(safe_rpm);
    Serial.print(" RPM... ");
    send_can_message(msg);
  }

  void set_position_speed(float degrees, float rpm, float acc) {
    if (!is_motor_on) {
      Serial.println("⚠ Motor is OFF - cannot set position/speed");
      return;
    }
    
    float safe_pos = constrain(degrees, P_MIN, P_MAX);
    float safe_rpm = constrain(rpm, V_MIN, V_MAX);
    float safe_acc = constrain(acc, A_MIN, A_MAX);
    
    CAN_message_t msg;
    msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS_SPD) << 8) | motor_id;
    msg.flags.extended = true;
    msg.len = 8;
    
    int32_t pos_scaled = (int32_t)(safe_pos * 10000.0f);
    int16_t rpm_scaled = (int16_t)(safe_rpm / 10.0f);
    int16_t acc_scaled = (int16_t)(safe_acc / 10.0f);
    
    msg.buf[0] = (pos_scaled >> 24) & 0xFF;
    msg.buf[1] = (pos_scaled >> 16) & 0xFF;
    msg.buf[2] = (pos_scaled >> 8) & 0xFF;
    msg.buf[3] = pos_scaled & 0xFF;
    msg.buf[4] = (rpm_scaled >> 8) & 0xFF;
    msg.buf[5] = rpm_scaled & 0xFF;
    msg.buf[6] = (acc_scaled >> 8) & 0xFF;
    msg.buf[7] = acc_scaled & 0xFF;
    
    Serial.print("Setting Pos: ");
    Serial.print(safe_pos);
    Serial.print("°, Speed: ");
    Serial.print(safe_rpm);
    Serial.print(" RPM, Acc: ");
    Serial.print(safe_acc);
    Serial.print("... ");
    send_can_message(msg);
  }

  void read_messages() {
    CAN_message_t msg;
    bool got_message = false;
    
    while (Can0.read(msg)) {
      uint8_t received_id = msg.id & 0xFF;
      if (received_id == motor_id) {
        if (!got_message) {
          Serial.println("📨 CAN Messages received:");
          got_message = true;
        }
        
        Serial.print("  Motor ");
        Serial.print(motor_id);
        Serial.print(" - ID: 0x");
        Serial.print(msg.id, HEX);
        Serial.print(", Data: ");
        for (int i = 0; i < msg.len; i++) {
          if (msg.buf[i] < 0x10) Serial.print("0");
          Serial.print(msg.buf[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
    }
  }

  void print_status() {
    Serial.println("=== MOTOR STATUS ===");
    Serial.print("Motor ID: ");
    Serial.println(motor_id);
    Serial.print("Status: ");
    Serial.println(is_motor_on ? "ON 🟢" : "OFF 🔴");
    Serial.print("Position limits: ");
    Serial.print(P_MIN);
    Serial.print("° to ");
    Serial.print(P_MAX);
    Serial.println("°");
    Serial.print("Speed limits: ");
    Serial.print(V_MIN);
    Serial.print(" to ");
    Serial.print(V_MAX);
    Serial.println(" RPM");
    Serial.println("===================");
  }
};

// =============================================================================
// 전역 변수
// =============================================================================
MotorTest motor(65);  // 모터 ID = 65

// =============================================================================
// 셋업 및 메인 루프
// =============================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {} // 3초 대기
  
  Serial.println("=======================================");
  Serial.println("🚀 AK60-6 Motor Test Program Started");
  Serial.println("=======================================");
  
  motor.init();
  motor.print_status();
  
  Serial.println("\n📋 Available Commands:");
  Serial.println("  'o' - Toggle motor ON/OFF");
  Serial.println("  'z' - Zero position (origin)");
  Serial.println("  'p' - Set position to 90°");
  Serial.println("  'n' - Set position to -90°");
  Serial.println("  's' - Set speed to 100 RPM");
  Serial.println("  'r' - Set speed to -100 RPM (reverse)");
  Serial.println("  'l' - Combined: 45°, 50 RPM, 20 acc");
  Serial.println("  'h' - Show this help");
  Serial.println("  'i' - Show motor status");
  Serial.println("  'x' - Emergency stop (turn OFF)");
  Serial.println("=======================================");
}

void loop() {
  // 명령어 처리
  if (Serial.available()) {
    char command = Serial.read();
    
    // 입력 버퍼 클리어
    while (Serial.available()) {
      Serial.read();
      delay(1);
    }
    
    Serial.print("\n>>> Command: '");
    Serial.print(command);
    Serial.println("'");
    
    switch (command) {
      case 'o':
        motor.toggle_power();
        break;
        
      case 'z':
        motor.zero_position();
        break;
        
      case 'p':
        motor.set_position(90.0f);
        break;
        
      case 'n':
        motor.set_position(-90.0f);
        break;
        
      case 's':
        motor.set_speed(100.0f);
        break;
        
      case 'r':
        motor.set_speed(-100.0f);
        break;
        
      case 'l':
        motor.set_position_speed(45.0f, 50.0f, 20.0f);
        break;
        
      case 'h':
        Serial.println("\n📋 Available Commands:");
        Serial.println("  'o' - Toggle motor ON/OFF");
        Serial.println("  'z' - Zero position (origin)");
        Serial.println("  'p' - Set position to 90°");
        Serial.println("  'n' - Set position to -90°");
        Serial.println("  's' - Set speed to 100 RPM");
        Serial.println("  'r' - Set speed to -100 RPM (reverse)");
        Serial.println("  'l' - Combined: 45°, 50 RPM, 20 acc");
        Serial.println("  'h' - Show this help");
        Serial.println("  'i' - Show motor status");
        Serial.println("  'x' - Emergency stop (turn OFF)");
        break;
        
      case 'i':
        motor.print_status();
        break;
        
      case 'x':
        Serial.println("🚨 EMERGENCY STOP!");
        motor.toggle_power();
        if (motor.is_motor_on) motor.toggle_power(); // 강제로 OFF
        break;
        
      case '\n':
      case '\r':
        // 개행문자 무시
        break;
        
      default:
        Serial.print("❌ Unknown command: '");
        Serial.print(command);
        Serial.println("' - Type 'h' for help");
        break;
    }
    
    Serial.println();
  }
  
  // CAN 메시지 수신 확인
  motor.read_messages();
  
  // CPU 부하 감소
  delay(10);
}