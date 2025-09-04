
#include "Arduino.h"
#include "FlexCAN_T4.h"
#include <cstdint>
#include <queue>


#if defined(ARDUINO_TEENSY36)
    static FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> Can0;
#elif defined(ARDUINO_TEENSY41)
    static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;
#endif

class CAN {
public:

  // 💡 싱글턴 패턴 수정: 메모리 누수 방지
  static CAN* getInstance() {
    static CAN instance;
    return &instance;
  }

  void send(CAN_message_t msg) {
    if (!Can0.write(msg)) {
    }
  }

  // 💡 read 함수 시그니처 수정
  bool read(CAN_message_t &msg) {
    return Can0.read(msg);
  }

private:

  CAN() {
    Can0.begin();
    Can0.setBaudRate(1000000); // 💡 생성자에서 BaudRate 설정
  }
};

#endif

namespace logic_micro_pins  // Teensy
{
// CAN Pins
const unsigned int can_rx_pin = 23;
const unsigned int can_tx_pin = 22;
const unsigned int motor_stop_pin = 9;

};

// 서보 모드용 CAN 명령어 ID
enum class CAN_PACKET_ID : uint8_t {
  SET_DUTY = 0,
  SET_CURRENT = 1,
  SET_CURRENT_BRAKE = 2,
  SET_RPM = 3,
  SET_POS = 4,
  SET_ORIGIN_HERE = 5,
  SET_POS_SPD = 6
};

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

class _Motor {
public:
  // 생성자에서 CAN ID를 직접 받음
  _Motor(uint8_t id)
    : _id(id), _is_on(false) {}
  virtual ~_Motor(){};
  virtual void read_data() = 0;
  virtual void set_position(float position) = 0;
  virtual void set_speed(float rpm) = 0;
  virtual void set_position_speed(float pos, float spd, float acc) = 0;
  virtual void on_off() = 0;
  virtual void zero() = 0;
protected:
  uint8_t _id;
  bool _is_on;
};


class _CANMotor : public _Motor
{
public:
    _CANMotor(uint8_t id);
    virtual ~_CANMotor(){};
    
    void read_data() override;
    void on_off() override;
    void zero() override;
    void set_position(float pos) override;
    void set_speed(float rpm) override;
    void set_position_speed(float pos, float spd, float acc) override;
protected:
    // Safety Limits
    float _P_MAX = 3600.0f; float _P_MIN = -3600.0f;
    float _V_MAX = 5000.0f; float _V_MIN = -5000.0f;
    float _A_MAX = 200.0f; float _A_MIN = 0.0f;
    float _I_MAX = 10.0f; float _I_MIN = -10.0f;
};

class AK60_v1_1 : public _CANMotor {
public:
    AK60_v1_1(uint8_t id) : _CANMotor(id) {}
};

_CANMotor::_CANMotor(uint8_t id)
  : _Motor(id) {}

void _CANMotor::on_off() {
  _is_on = !_is_on;
  Serial.print("Motor ");
  Serial.print(_id);
  Serial.print(" is now ");
  Serial.println(_is_on ? "ON" : "OFF");
}

void _CANMotor::zero() {
  CAN_message_t msg;
  msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_ORIGIN_HERE) << 8) | _id;
  msg.flags.extended = true; msg.len = 1; msg.buf[0] = 0;
  if (_is_on) {
    CAN::getInstance()->send(msg);
    // read_data(); // 💡 이 부분은 필요에 따라 주석 처리하거나 제거
  }
}

void _CANMotor::read_data() {
  CAN_message_t msg;
  if (CAN::getInstance()->read(msg)) {
    if (msg.id == _id) {
        Serial.println("CAN Message Received!");
    }
  }
}

void _CANMotor::set_position(float pos) {
  float constrained_pos = constrain(pos, _P_MIN, _P_MAX);
  CAN_message_t msg;
  msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS) << 8) | _id;
  msg.flags.extended = true; msg.len = 4;
  int32_t pos_scaled = (int32_t)(constrained_pos * 10000.0f);
  msg.buf[0] = pos_scaled >> 24; msg.buf[1] = pos_scaled >> 16;
  msg.buf[2] = pos_scaled >> 8; msg.buf[3] = pos_scaled;
  if (_is_on) { CAN::getInstance()->send(msg); }
}

void _CANMotor::set_speed(float rpm) {
  float constrained_rpm = constrain(rpm, _V_MIN, _V_MAX);
  CAN_message_t msg;
  msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_RPM) << 8) | _id;
  msg.flags.extended = true; msg.len = 4;
  int32_t rpm_scaled = (int32_t)constrained_rpm;
  msg.buf[0] = rpm_scaled >> 24; msg.buf[1] = rpm_scaled >> 16;
  msg.buf[2] = rpm_scaled >> 8; msg.buf[3] = rpm_scaled;
  if (_is_on) { CAN::getInstance()->send(msg); }
}

void _CANMotor::set_position_speed(float pos, float spd, float acc) {
  float constrained_pos = constrain(pos, _P_MIN, _P_MAX); float constrained_spd = constrain(spd, _V_MIN, _V_MAX); float constrained_acc = constrain(acc, _A_MIN, _A_MAX);
  CAN_message_t msg;
  msg.id = (static_cast<uint32_t>(CAN_PACKET_ID::SET_POS_SPD) << 8) | _id;
  msg.flags.extended = true; msg.len = 8; int32_t pos_scaled = (int32_t)(constrained_pos * 10000.0f);
  int16_t spd_scaled = (int16_t)(constrained_spd / 10.0f); int16_t acc_scaled = (int16_t)(constrained_acc / 10.0f);
  msg.buf[0] = pos_scaled >> 24; msg.buf[1] = pos_scaled >> 16; msg.buf[2] = pos_scaled >> 8; msg.buf[3] = pos_scaled;
  msg.buf[4] = spd_scaled >> 8; msg.buf[5] = spd_scaled;
  msg.buf[6] = acc_scaled >> 8; msg.buf[7] = acc_scaled;
  if (_is_on) { CAN::getInstance()->send(msg); }
}


// ----------------------------------------------------
// Main Sketch
// ----------------------------------------------------
AK60_v1_1 my_motor(65);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Motor Test Sketch Started.");
  Serial.println("Send 'o' to toggle ON/OFF, 'z' to zero, 'p' to set Position, 's' to set Speed. 'l' to set position speed");
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        switch (command) {
          case 'o': my_motor.on_off(); break;
            case 'z': my_motor.zero(); break;
            case 'p': my_motor.set_position(100.0f); break;
            case 's': my_motor.set_speed(100.0f); break;
    }
  }
  my_motor.read_data();
}