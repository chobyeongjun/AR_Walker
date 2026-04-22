# 새 보드 제작 마스터 플랜

> **목표**: AR_Walker Teensy 기반 시스템을 → STM32 + Jetson 아키텍처로 발전시켜 새 보드 제작

---

## 현재 시스템 파악 (AR_Walker 레포 분석 결과)

| 항목 | 현재 (Teensy 기반) |
|------|--------------------|
| 메인 MCU | Teensy 4.1 (Cortex-M7, 600MHz) |
| 통신 MCU | Arduino Nano 33 BLE |
| 모터 | AK-series CAN 모터 (AK10/60/70/80) |
| 제어 | 어드미턴스 (입각기) + PID 위치/속도 (유각기) |
| 루프 주파수 | 500Hz |
| 센서 | 로드셀, IMU (BNO055), FSR |
| 인터페이스 | CAN (모터), BLE (폰), UART (MCU간) |

---

## Phase 0: 아키텍처 결정 (1주)

### 핵심 질문: STM32 단독? Jetson 포함?

**옵션 A - STM32 단독 보드**
```
STM32H7 (Cortex-M7, 480MHz)
├── CAN-FD → AK 모터
├── SPI/I2C → IMU, Loadcell ADC
├── UART → BLE 모듈 또는 Jetson
└── SD카드 → 데이터 로깅
```
- 장점: 진짜 RTOS, 500Hz 이상 안정 제어, 저전력, 경량
- 단점: ML/CV 불가, Python 개발 어려움

**옵션 B - STM32 + Jetson 듀얼 (권장)**
```
Jetson Orin Nano (고수준)          STM32H7 (저수준)
├── 보행 패턴 분류                  ├── 500Hz 제어 루프
├── AI 어시스턴스                   ├── CAN 모터 직접 제어
├── 카메라/비전                     ├── 센서 읽기 (로드셀, IMU)
└── UART/SPI → STM32 명령         └── 상태 → Jetson 전송
```
- 장점: 현재 Teensy+Nano 구조와 유사, 확장성 ↑
- 단점: 복잡도 증가, 비용 증가

**옵션 C - Jetson 단독 (실험적)**
```
Jetson + PREEMPT_RT 패치
├── SocketCAN → AK 모터 (위치 명령 ~200Hz)
├── Python 제어 루프
└── 향후 AI 통합 용이
```
- 장점: 단일 플랫폼, 개발 편의
- 단점: Linux 지터 문제 → 500Hz 어드미턴스에 위험

> **의견**: "Position 명령만 던지는" 용도는 Jetson 단독으로 충분.
> 단, 어드미턴스 제어 (힘 피드백, 500Hz)는 STM32 공동 필수.

---

## Phase 1: STM32 기초 공부 (2주)

### 1-1. STM32H7 vs 현재 Teensy 비교

| 항목 | Teensy 4.1 | STM32H7B3I-DK |
|------|-----------|----------------|
| Core | Cortex-M7 600MHz | Cortex-M7 280MHz |
| FPU | DP-FPU | DP-FPU |
| CAN | FlexCAN | FDCAN (CAN-FD 지원) |
| ADC | 12bit | 16bit |
| IDE | Arduino | STM32CubeIDE / CubeMX |
| RTOS | 없음 (단순 loop) | FreeRTOS 통합 용이 |

### 1-2. 학습 순서

1. **STM32CubeIDE + CubeMX 설치 및 클럭 설정**
   - H7 클럭 트리 이해 (핵심: APB1이 CAN에 영향)
   
2. **GPIO + 타이머 인터럽트**
   - TIM → 500Hz 인터럽트로 제어 루프 트리거
   - Teensy의 `IntervalTimer`와 동일 기능
   
3. **FDCAN 드라이버 구현**
   - HAL_FDCAN 사용 → AK 모터 CAN 프레임 송수신
   - 레퍼런스: AR_Walker의 `CAN.h`, `Motor.cpp`
   
4. **ADC DMA** (로드셀 읽기)
   - 12/16bit ADC → 로드셀 mV → 힘(N) 변환
   
5. **I2C** (IMU BNO055 연결)

6. **FreeRTOS 태스크 구조**
   ```
   Task_Control (500Hz, 최고 우선순위)
   Task_Sensor (500Hz, 높은 우선순위)
   Task_Comm   (100Hz, 보통 우선순위)
   Task_Logger (10Hz, 낮은 우선순위)
   ```

### 1-3. skiro 활용 (Phase 1)

- 각 드라이버 파일 작성 시: `skiro_analyze_complexity` 실행
- CAN 프레임 파싱 버그 발생 시: `skiro_record_problem` 즉시 기록
- 에러 해결 시: `skiro_record_solution` + `promote`

---

## Phase 2: CAN 모터 드라이버 포팅 (2주)

### 2-1. AK 모터 CAN 프로토콜

AR_Walker의 `Motor.cpp`에서 추출한 CAN 명령 구조:

```c
// MIT 모드 (토크 직접 제어)
// ID: 모터 CAN ID
// 데이터: position, speed, kp, kd, torque (각 12/16bit 패킹)

// SET_POS 명령 (단순 위치)
// 데이터: [0x0A, 0x00, pos_high, pos_low, ...]
```

### 2-2. 구현 파일 구조

```
stm_board/
├── Core/
│   ├── Src/
│   │   ├── main.c
│   │   ├── can_motor.c      ← Motor.cpp 포팅
│   │   ├── controller.c     ← Controller.cpp 포팅
│   │   ├── loadcell.c
│   │   └── imu.c
│   └── Inc/
│       ├── can_motor.h
│       ├── controller.h
│       └── config.h
├── Drivers/
│   └── STM32H7xx_HAL_Driver/
└── BOARD_PLAN.md
```

### 2-3. 검증 순서

1. CAN 루프백 테스트 (모터 없이 자기 자신 수신 확인)
2. 모터 Enable/Disable 명령 확인
3. SET_ORIGIN (원점 설정)
4. SET_POS 단순 위치 명령
5. MIT 모드 (토크 직접 제어)

> **skiro safety gate**: 모터 연결 전 반드시 `skiro_safety_gate_create` 실행

---

## Phase 3: 제어기 포팅 (2주)

### 3-1. 어드미턴스 제어 STM32 포팅

AR_Walker `AdmittanceController`의 핵심:

```c
// 어드미턴스 방정식
// a = (F_error - D*v - K*x) / M
// v += a * dt
// x += v * dt
// cmd_position = baseline + x

typedef struct {
    float M;  // 가상 질량 [kg]
    float D;  // 댐핑 [Ns/m]
    float K;  // 강성 [N/m]
    float v;  // 현재 속도
    float x;  // 현재 변위
    float dt; // 1/500Hz = 0.002s
} AdmittanceState;
```

### 3-2. 보행 위상 판단

```c
// AR_Walker의 SideData::is_swing_phase() 기반
// FSR 발바닥 압력 → STANCE/SWING 판단
// Loadcell 힘 → 어드미턴스 입력
```

### 3-3. 미해결 이슈 (skiro 기록 확인)

- `Kd 유한차분 구현` (skiro 미해결)
  → 진동 발생 시 적용, 지금은 보류
- `오차 → 토크 → F_desired 흐름` (skiro 미해결)
  → 어드미턴스 출력은 위치 명령, 토크 직접 제어와 다름
  → AK 모터 MIT 모드 사용 시 이 흐름 필요

---

## Phase 4: Jetson 연동 (선택, 1주)

### 4-1. Jetson + STM32 통신

```
Jetson (Ubuntu + Python)
│
│ UART (115200 or 921600 baud)
│ 또는 SPI (고속)
│
STM32H7
```

### 4-2. Jetson의 역할

- 고수준 보행 모드 전환 (STM32에 명령)
- 카메라 기반 자세 추정
- 데이터 기록/분석 (SD카드 대신)
- Python GUI (기존 AR_Walker GUI 재사용)

### 4-3. Jetson 단독 CAN 제어 (확인된 사항)

- 간단한 위치 명령: **가능** (이미 확인됨)
- 500Hz 어드미턴스 제어: **권장하지 않음** (Linux 지터)
- 해결책: PREEMPT_RT 패치 + CPU isolation (`isolcpus`)

---

## Phase 5: PCB 설계 (3주)

### 5-1. 보드 요구사항

| 기능 | 부품 |
|------|------|
| MCU | STM32H7B3 또는 H743 |
| CAN | TJA1051 (x2, 좌우 독립) |
| ADC | ADS1256 (24bit, 로드셀) 또는 STM32 내장 |
| IMU | BNO055 (I2C) |
| 전원 | LM2596 (배터리 → 3.3V/5V) |
| 디버그 | SWD (10핀 커넥터) |
| 통신 | UART → Jetson, USB CDC |

### 5-2. PCB 툴

- KiCad 7 (오픈소스, 권장)
- AR_Walker의 `Walker_Main/Hardware/PCB/` 참조

---

## skiro 통합 워크플로우

### 각 Phase 시작 시
```bash
skiro-learnings list --last 5  # 이전 이슈 확인
```

### 코드 파일 작성 시
```
skiro_analyze_complexity <파일>
# tier: fast/partial/full 확인 후 그에 맞게 검토
```

### 하드웨어 테스트 전 (필수)
```
skiro_safety_gate_create
# CRITICAL 이슈 0개 확인 후에만 전원 인가
```

### 버그/문제 발생 즉시
```
skiro_record_problem --problem "..." --category control --severity WARNING
```

### 문제 해결 시
```
skiro_record_solution --solution "..." --keyword "..."
```

---

## 전체 타임라인

| 주차 | 작업 |
|------|------|
| 1주 | Phase 0: 아키텍처 결정, 부품 주문 |
| 2-3주 | Phase 1: STM32 기초 (GPIO, 타이머, FDCAN) |
| 4-5주 | Phase 2: CAN 모터 드라이버 구현+검증 |
| 6-7주 | Phase 3: 어드미턴스 제어기 포팅+튜닝 |
| 8주 | Phase 4: Jetson 연동 (선택) |
| 9-11주 | Phase 5: PCB 설계 및 제작 |

---

## 즉시 시작할 것들

1. [ ] STM32CubeIDE 설치
2. [ ] 개발 보드 주문: **NUCLEO-H743ZI** (CAN 내장, SWD 디버거 포함, ~$25)
3. [ ] AR_Walker `Motor.cpp`, `CAN.h` 정독 → STM32 포팅 대상 명확화
4. [ ] skiro `Kd 유한차분` 이슈 해결 전략 수립
