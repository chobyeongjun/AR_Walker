# Step 5: 기존 코드 구조 파악

> 새 시스템을 만들기 전에, 지금 있는 코드가 어떻게 동작하는지 이해하자.
> 여기서 재사용할 로직을 찾고, Elmo + STM32로 포팅할 부분을 파악한다.

---

## 1. 전체 코드 구조

```
Walker_Main/
├── Main.ino              ← 프로그램 시작점 (setup + loop)
├── src/
│   ├── Config.h          ← 시스템 설정값 (500Hz, 핀, 보정값)
│   ├── Exo.cpp/h         ← 메인 제어 클래스 (run 함수 = 제어 루프)
│   ├── Side.cpp/h        ← 좌/우측 관리
│   ├── Joint.cpp/h       ← 관절 제어 (무릎, 발목)
│   ├── Motor.cpp/h       ← ★ 모터 CAN 통신 (핵심!)
│   ├── MotorData.cpp/h   ← 모터 상태 데이터
│   ├── CAN.h             ← CAN 버스 설정 (싱글톤)
│   ├── IMU.cpp/h         ← EBIMU IMU 데이터 처리
│   ├── Loadcell.cpp/h    ← 로드셀 센서
│   ├── ExoBLE.cpp/h      ← BLE 통신
│   ├── UARTHandler.cpp/h ← UART 통신 (Teensy↔Nano)
│   ├── Controller.cpp/h  ← 제어 알고리즘 (토크 계산)
│   └── ExoData.cpp/h     ← 전체 시스템 공유 데이터
```

---

## 2. 데이터 흐름 (한 루프 사이클)

```
500Hz 루프 (매 2ms):
┌──────────────────────────────────────────────────────┐
│                                                      │
│  1. IMU 데이터 수신    ← Serial4 (UART, 115200bps)  │
│     → pitch, gyro_y, battery 저장                    │
│                                                      │
│  2. 로드셀 읽기        ← ADC (12비트)               │
│     → 힘 값 저장                                     │
│                                                      │
│  3. left_side.run_side()                             │
│     └→ joint.run_joint()                             │
│        └→ controller.calc()  → 토크 계산             │
│        └→ motor.set_position() → CAN 명령 전송       │
│        └→ motor.read_data()   → CAN 응답 수신        │
│                                                      │
│  4. right_side.run_side()  (같은 과정)               │
│                                                      │
│  5. UART로 Nano에 데이터 전송 → BLE → GUI            │
│                                                      │
└──────────────────────────────────────────────────────┘
```

---

## 3. 핵심 파일 분석

### 3-1. Config.h — 시스템 설정
> 경로: `Walker_Main/src/Config.h`

```cpp
#define LOOP_FREQ_HZ 500       // ★ 제어 루프 주파수
#define LOOP_TIME_TOLERANCE 0.1 // 루프 시간 허용 오차 (10%)

namespace logging {
    const int baud_rate = 115200;  // 디버그 시리얼 속도
}

namespace loadcell_calibration {
    const float AI_CNT_TO_V = 3.3 / 4096;  // 12비트 ADC → 전압 변환
    // 각 관절별 보정값 (bias, sensitivity)
}
```

**STM32 포팅 시**:
- `LOOP_FREQ_HZ` → TIM 인터럽트 주기로 변환: 1/500 = 2ms
- ADC 해상도: 12비트 → 16비트로 업그레이드 (INA828)
- 보드레이트 등 통신 설정 동일하게 유지 가능

---

### 3-2. CAN.h — CAN 버스 관리
> 경로: `Walker_Main/src/CAN.h`

```cpp
// 싱글톤 패턴 — CAN 버스 인스턴스 하나만 존재
class CAN {
    static CAN* getInstance();  // 어디서든 같은 인스턴스 접근
    void send(CAN_message_t msg);  // 메시지 송신
    CAN_message_t read();          // 메시지 수신
private:
    CAN() {
        Can0.begin();
        Can0.setBaudRate(1000000);  // ★ 1Mbps (Classic CAN)
    }
};
```

**핵심 포인트**:
- 싱글톤 패턴: `CAN::getInstance()` 로 어디서든 접근
- Teensy용 FlexCAN_T4 라이브러리 사용
- **1Mbps Classic CAN** (CAN FD 아님)

**STM32 포팅 시**:
- FlexCAN_T4 → STM32 HAL FDCAN 드라이버로 교체
- 1Mbps → CAN FD (중재부 1Mbps + 데이터부 8Mbps)
- 송수신 함수 인터페이스는 비슷하게 유지 가능

---

### 3-3. Motor.h — 모터 클래스 구조 ★★★
> 경로: `Walker_Main/src/Motor.h`

```
클래스 계층:
  _Motor (추상 기본 클래스)
    ├── NullMotor (모터 없는 관절용)
    └── _CANMotor (CAN 모터 기본 클래스)
          ├── AK10
          ├── AK60_v1_1
          ├── AK70
          └── AK80
```

**_Motor 클래스 (기본)**:
```cpp
class _Motor {
    virtual void read_data() = 0;       // 모터 데이터 읽기
    virtual void set_position(float) = 0; // 위치 제어
    virtual void set_speed(float) = 0;    // 속도 제어
    virtual void on_off() = 0;            // 모터 ON/OFF
    virtual void zero() = 0;              // 영점 설정
    virtual float get_Kt() = 0;           // 토크 상수 반환

    config_defs::joint_id _id;  // 관절 ID
    bool _is_left;               // 좌/우 구분
    ExoData *_data;              // 공유 데이터 포인터
    MotorData *_motor_data;      // 모터별 데이터
    float _Kt;                   // 토크 상수
};
```

**_CANMotor 클래스 (CAN 모터)**:
```cpp
class _CANMotor : public _Motor {
    // 안전 한계값
    float _P_MAX, _P_MIN;  // 위치 한계 (도)
    float _V_MAX, _V_MIN;  // 속도 한계 (RPM)
    float _I_MAX, _I_MIN;  // 전류 한계 (A)

    const uint32_t _timeout = 500;  // CAN 응답 타임아웃 (μs)

    // 모터 응답 감시 (전류 분산으로 스톨 감지)
    std::queue<float> _measured_current;
    const float _variance_threshold = 0.01;
};
```

**STM32 포팅 시**:
- `_Motor` 추상 클래스 구조 **재사용 가능**
- `_CANMotor` → `_ElmoMotor`로 변경
- AK 시리즈 CAN 프로토콜 → **Elmo CANopen 프로토콜**로 교체
- 안전 한계값, 타임아웃 로직은 그대로 활용

---

### 3-4. Motor.cpp — CAN 모터 통신 로직 ★★★
> 경로: `Walker_Main/src/Motor.cpp` (361줄)

#### `read_data()` — 모터 응답 읽기 (101~152줄)
```cpp
void _CANMotor::read_data() {
    // 타임아웃(500μs) 내에 자기 모터 ID의 CAN 메시지를 찾음
    do {
        CAN_message_t msg = can->read();
        if (msg.id == _motor_data->id) {
            // 바이트 파싱 → 위치, 속도, 전류, 온도, 에러
            _motor_data->p = (float)p_int * 0.1f;   // 위치 (도)
            _motor_data->v = (float)v_int * 10.0f;   // 속도 (RPM)
            _motor_data->i = (float)i_int * 0.01f;   // 전류 (A)
            _motor_data->temperature = temp_int;
            _motor_data->error = error_code_raw;
            return;
        }
    } while ((micros() - start) < _timeout);

    _handle_read_failure();  // 타임아웃 시 실패 처리
}
```

**핵심 패턴**: 전송 → 타임아웃 내 응답 대기 → 파싱 → 실패 처리
→ Elmo 포팅 시 이 패턴 동일하게 사용, 파싱 부분만 CANopen 형식으로 변경

#### `set_position()` — 위치 명령 (209~234줄)
```cpp
void _CANMotor::set_position(float pos) {
    float constrained_pos = constrain(pos, _P_MIN, _P_MAX);  // 안전 한계

    CAN_message_t msg;
    msg.id = (SET_POS << 8) | motor_id;  // 확장 ID 사용
    msg.flags.extended = 1;
    msg.len = 4;
    // int32로 스케일링 후 바이트로 분해
    msg.buf[0~3] = pos_scaled 각 바이트;

    can->send(msg);     // 명령 전송
    read_data();        // 응답 읽기
    check_response();   // 응답 검증
}
```

**핵심 패턴**: 안전 한계 체크 → 값 스케일링 → CAN 메시지 구성 → 전송 → 응답 확인
→ Elmo에서는 CANopen SDO/PDO 메시지 형식으로 변경 필요

#### `check_response()` — 모터 상태 감시 (184~207줄)
```cpp
// 최근 25개 전류값의 분산을 계산
// 분산이 너무 낮으면 → 모터가 멈춤 (스톨) 가능성
if (variance < 0.01) {
    // 경고 로그
}
```

**재사용 가능**: 전류 분산 기반 스톨 감지 로직은 모터 종류와 무관

---

### 3-5. Exo.cpp — 메인 제어 루프 ★★
> 경로: `Walker_Main/src/Exo.cpp` (367줄)

#### `run()` — 500Hz 제어 루프 (134~181줄)
```cpp
bool Exo::run() {
    // 시간 경과 체크 (2ms = 500Hz)
    if (delta_t >= lower_bound) {

        // 1. E-stop 체크
        if (data->estop) {
            left_side.disable_motors();
            right_side.disable_motors();
        }

        // 2. IMU 데이터 처리
        _process_imu_data();

        // 3. 좌/우 제어 실행
        left_side.run_side();   // → joint.run_joint() → motor.set_position()
        right_side.run_side();

        // 4. 상태 LED 업데이트
        status_led.update(data->get_status());

        // 5. UART 통신 (Nano ↔ Teensy)
        UART_msg_t msg = handler->poll(timeout);
        UART_command_utils::handle_msg(handler, data, msg);

        // 6. 실시간 데이터 전송 (BLE용)
        if (rt_delta_t >= BLE_times::_real_time_msg_delay) {
            // 데이터 패킹 & 전송
        }

        delta_t = 0;
        return true;  // 루프 실행됨
    }
    return false;  // 아직 시간 안 됨
}
```

**STM32 포팅 시**:
- 시간 체크 → **TIM 인터럽트**로 교체 (더 정확)
- `_process_imu_data()` → UART DMA 수신으로 교체
- `left/right_side.run_side()` → 제어 알고리즘 구조 재사용
- UART 통신 → STM32↔ESP32 SPI + STM32↔Jetson UART

#### `_process_imu_data()` — IMU 패킷 파싱 (32~132줄)
```
Serial4에서 바이트 단위로 수신
→ SOP(시작 바이트) 2개 찾기
→ 패킷 완성되면 ID로 구분 (0~3: 좌발목/우발목/좌무릎/우무릎)
→ 체크섬 검증 후 데이터 저장
→ 타임아웃/에러율 관리
```

**재사용 가능**: EBIMU 패킷 파싱 로직은 STM32에서도 동일하게 사용

---

## 4. 포팅 전략 요약

### 그대로 재사용 (로직 동일)
| 기존 코드 | 재사용 부분 |
|-----------|-----------|
| `_Motor` 클래스 구조 | 추상 클래스, 가상 함수 패턴 |
| 안전 한계 체크 | `constrain()`, 한계값 검증 |
| 스톨 감지 | 전류 분산 감시 로직 |
| IMU 패킷 파싱 | SOP + 체크섬 + ID 구분 |
| 제어 루프 구조 | 센서 읽기 → 계산 → 명령 → 통신 |
| 에러 처리 패턴 | 타임아웃, 에러 카운트, 상태 머신 |

### 변경 필요 (프로토콜/하드웨어 차이)
| 기존 | 변경 | 이유 |
|------|------|------|
| FlexCAN_T4 | STM32 HAL FDCAN | MCU 교체 |
| AK CAN 프로토콜 | Elmo CANopen | 모터 드라이버 교체 |
| INA125U + 12비트 ADC | INA828 + 16비트 ADC | 앰프 업그레이드 + ADC 해상도 향상 |
| Arduino Serial | STM32 HAL UART + DMA | 성능 향상 |
| BLE (Nano) | SPI → ESP32 → Wi-Fi/BLE | 통신 아키텍처 변경 |
| 시간 체크 루프 | TIM 인터럽트 | 정확한 500Hz 보장 |

---

## 5. 핵심 개념 정리

### CAN 메시지 ID 구조 (기존 AK 모터)
```
확장 ID (29비트):
  [명령 코드 (상위 8비트)] [모터 ID (하위 8비트)]

예: SET_POS(0x04) + 모터 ID(0x01)
  → msg.id = 0x0401
```

### 데이터 스케일링
```
모터에서 받은 raw 데이터를 실제 단위로 변환:
  위치: raw × 0.1 → 도 (degrees)
  속도: raw × 10  → RPM
  전류: raw × 0.01 → A (암페어)
```

### 싱글톤 패턴 (CAN)
```
프로그램 전체에서 CAN 버스 인스턴스가 딱 하나만 존재
→ 어디서든 CAN::getInstance()로 같은 인스턴스 접근
→ 여러 모터가 같은 CAN 버스를 공유
```

---

## 연습: 코드 읽기

### 문제 1: read_data() 이해
> `_CANMotor::read_data()`에서 타임아웃이 500μs인 이유는?

<details>
<summary>정답 보기</summary>

```
500Hz 제어 루프 = 매 2000μs (2ms)
그 안에 여러 모터의 읽기/쓰기를 모두 처리해야 함

모터 1개당 500μs 대기 × 4개 모터 = 2000μs → 딱 맞음
→ 타임아웃을 더 길게 하면 제어 루프를 놓칠 수 있음
```
</details>

### 문제 2: Elmo 포팅 시 뭘 바꿔야 하나?
> `set_position()` 함수를 Elmo용으로 바꾸려면 어떤 부분이 변경되나?

<details>
<summary>정답 보기</summary>

```
변경할 부분:
1. CAN 메시지 ID → Elmo CANopen SDO/PDO ID로 변경
2. 데이터 포맷 → CANopen 표준 형식으로 인코딩
3. 스케일링 팩터 → Elmo 매뉴얼의 단위에 맞게 변경

유지할 부분:
1. constrain()으로 안전 한계 체크
2. send → read → check 패턴
3. 에러 처리 로직
```
</details>

---

## 다음 단계
→ Step 6: STM32CubeIDE 세팅 + LED 블링크 (실제 하드웨어 작업!)
