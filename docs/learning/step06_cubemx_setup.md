# Step 6: STM32CubeIDE 설정 (보드 없이 공부)

> 보드 없이 PC에서 CubeIDE를 설치하고, 프로젝트를 만들어보자.
> 실제 칩이 없어도 설정 연습과 코드 생성까지 가능하다!

---

## 1. STM32CubeIDE 설치

### CubeIDE가 뭐야?

```
CubeIDE = ST가 만든 STM32 전용 개발 도구 (무료!)

하는 일:
  1. CubeMX — 칩 설정 GUI (핀 배정, 클럭 설정을 마우스 클릭으로!)
  2. 코드 편집기 — C 코드 작성
  3. 컴파일러 — 코드를 STM32가 이해하는 기계어로 변환
  4. 디버거 — 코드 실행하면서 한 줄씩 확인 (보드 필요)

비유:
  아두이노 IDE = 간단한 메모장 + 업로드 버튼
  CubeIDE = 전문 개발 환경 + 칩 설정 GUI + 디버거 풀세트
```

### 설치 방법

```
1. ST 공식 사이트 접속
   → st.com/en/development-tools/stm32cubeide.html

2. 운영체제에 맞는 버전 다운로드
   → Windows / macOS / Linux
   → 용량: 약 2~3GB

3. 설치 중 "ST-Link 드라이버도 설치할까요?" → Yes!
   → 나중에 보드 연결할 때 필요

4. 설치 완료 후 실행
   → Workspace 경로 묻는 창 뜸
   → 원하는 폴더 지정 (예: C:\STM32_Projects)

⚠️ 회원가입 필요 (무료)
   → ST 계정 만들어야 다운로드 가능
   → 이메일 인증만 하면 됨
```

---

## 2. 새 프로젝트 생성

### Step by Step

```
① File → New → STM32 Project

② Target Selection (칩 선택 화면이 뜸):
   검색창에 "STM32H743VI" 입력
   → STM32H743VIT6 선택 ★

   V = 100핀 LQFP 패키지
   I = 산업용 온도 범위
   T = 패키지 타입
   6 = 주문 코드

③ Project Name 입력: "AR_Walker_STM32"

④ Targeted Language: C

⑤ Targeted Binary Type: Executable

⑥ Finish 클릭
   → CubeMX 화면이 열림 (칩 그림이 보임!)
```

### CubeMX 화면 구성

```
┌──────────────────────────────────────────────────┐
│ [메뉴바]                                          │
├──────────┬───────────────────────────────────────┤
│          │                                       │
│ 카테고리  │         칩 그림 (핀맵)                  │
│          │                                       │
│ □ System │    ┌─────────────────────┐            │
│ □ Analog │    │ ●  ●  ●  ●  ●  ●  │            │
│ □ Timers │    │●                  ●│            │
│ □ Conn.  │    │●    STM32H743     ●│            │
│ □ Multim.│    │●                  ●│            │
│          │    │ ●  ●  ●  ●  ●  ●  │            │
│          │    └─────────────────────┘            │
│          │                                       │
│          │    ← 핀을 클릭하면 기능 선택 가능!       │
├──────────┴───────────────────────────────────────┤
│ [Configuration] [Clock] [Project Manager]         │
└──────────────────────────────────────────────────┘

왼쪽: 카테고리별 설정 (RCC, UART, CAN, SPI 등)
가운데: 칩 핀맵 (핀 클릭으로 기능 배정)
아래 탭: Configuration(설정) / Clock(클럭) / Project Manager(프로젝트)
```

---

## 3. RCC 클럭 설정 (첫 번째 설정!)

### 왜 이걸 먼저 해야 하나?

```
클럭 = STM32의 심장박동
  → 클럭이 없으면 CPU, UART, CAN, SPI 전부 동작 안 함
  → 먼저 심장박동을 설정해야 나머지를 설정할 수 있음!
```

### 설정 방법

```
① 왼쪽 카테고리에서 "System Core" → "RCC" 클릭

② High Speed Clock (HSE): "Crystal/Ceramic Resonator" 선택 ★
   → 외부 수정 발진기 사용 (정밀한 클럭)
   → CAN FD 통신에 필수! (오차 0.5% 이하 필요)

③ Low Speed Clock (LSE): "Disable"
   → RTC(실시간 시계) 안 쓰니까 비활성화

④ 나머지 체크박스: 전부 체크 해제
   → Master Clock Output: 불필요
   → Audio Clock Input: 불필요
```

### 클럭 트리 설정 (Clock Configuration 탭)

```
아래 "Clock Configuration" 탭 클릭 → 클럭 트리 화면

이런 그림이 보임:

[HSE 25MHz] → [PLL] → [SYSCLK 480MHz] → [각 버스로 분배]

설정할 것:
  ① HSE 주파수 입력: 25 (MHz) ← 보드의 crystal 주파수
     (WeAct = 25MHz, Nucleo = 25MHz가 일반적)

  ② SYSCLK 설정: 480 (MHz) ← 최대 성능!

  ③ HCLK: 240MHz (자동 계산됨)

  ④ APB1/APB2 클럭: 120MHz (자동 계산됨)
     → UART, CAN FD, SPI는 이 클럭을 사용

설정 방법 (쉬움!):
  → SYSCLK에 "480" 입력하고 Enter
  → CubeMX가 나머지를 자동으로 계산해줌!
  → "Resolve Clock Issues" 버튼 누르면 자동 최적화
```

### PLL이 뭐야?

```
PLL = Phase-Locked Loop (위상 고정 루프)
    = 낮은 주파수를 높은 주파수로 뻥튀기하는 회로

[Crystal 25MHz] → [PLL ×19.2배] → [480MHz]

왜 Crystal을 25MHz로 쓰고 PLL로 올리나?
  → 480MHz crystal은 존재하지 않음 (물리적 한계)
  → 낮은 주파수 crystal이 안정적이고 저렴
  → PLL이 정밀하게 체배(곱하기)해줌
```

---

## 4. 핀 배정 — AR Walker에 필요한 것들

### 우리가 배정해야 할 핀 목록

```
CAN FD × 2채널 (Elmo 모터 드라이버 2개):
  → FDCAN1: PD0 (RX), PD1 (TX)
  → FDCAN2: PB12 (RX), PB13 (TX)
  → 각 채널 = 핀 2개 (TX, RX)

UART × 3채널:
  → USART1: PA9 (TX), PA10 (RX)  — Jetson 통신
  → USART2: PD5 (TX), PD6 (RX)  — EBIMU RF 수신
  → USART3: PB10 (TX), PB11 (RX) — BMS 통신

SPI × 1채널 (ESP32-S3):
  → SPI1: PA5 (SCK), PA6 (MISO), PA7 (MOSI)
  → + PA4 (NSS/CS) — 칩 선택 핀

ADC × 4채널 (INA828 로드셀 4개):
  → ADC1_CH0: PA0
  → ADC1_CH1: PA1
  → ADC1_CH2: PA2
  → ADC1_CH3: PA3

GPIO (일반 입출력):
  → LED 상태 표시용: 1~2핀
  → 비상 정지 버튼: 1핀

타이머 (500Hz 제어 루프용):
  → TIM2 또는 TIM6 — 인터럽트 주기 생성
```

### CubeMX에서 핀 배정하는 방법

```
방법 1: 칩 그림에서 핀 직접 클릭
  → 핀 클릭 → 기능 목록 뜸 → 원하는 기능 선택
  → 예: PD0 클릭 → "FDCAN1_RX" 선택

방법 2: 왼쪽 카테고리에서 설정 (추천 ★)
  → Connectivity → FDCAN1 클릭
  → Mode: "Activated" 선택
  → 자동으로 핀이 배정됨!
  → 핀을 바꾸고 싶으면 Ctrl+클릭으로 다른 핀으로 이동 가능
```

### FDCAN1 설정 예시 (Elmo 모터 1)

```
왼쪽 카테고리 → Connectivity → FDCAN1

Mode: "Activated"

Parameter Settings:
  Frame Format:        FD CAN ← 일반 CAN 아니고 FD!
  Mode:                Normal
  Auto Retransmission: Enable
  Nominal Bit Rate:    1000000 (1Mbps) ← CAN 표준 속도
  Data Bit Rate:       5000000 (5Mbps) ← FD 데이터 구간 고속!

→ 핀이 자동 배정됨: PD0 (RX), PD1 (TX)
→ 칩 그림에서 PD0, PD1이 초록색으로 바뀜!
```

### UART 설정 예시 (Jetson 통신)

```
왼쪽 → Connectivity → USART1

Mode: "Asynchronous" (비동기 = 일반 UART)

Parameter Settings:
  Baud Rate:    115200 (또는 필요에 따라 변경)
  Word Length:  8 Bits
  Stop Bits:    1
  Parity:       None

→ 핀 자동 배정: PA9 (TX), PA10 (RX)

⚠️ Jetson과 STM32는 전압이 다를 수 있음:
  STM32: 3.3V 로직
  Jetson: 3.3V 로직
  → 같으니까 직접 연결 OK!
```

### SPI 설정 예시 (ESP32 통신)

```
왼쪽 → Connectivity → SPI1

Mode: "Full-Duplex Master" ← STM32가 마스터!

Parameter Settings:
  Data Size:    8 Bits
  First Bit:    MSB First
  Prescaler:    적절히 (클럭 속도 조절)

→ 핀 자동 배정: PA5 (SCK), PA6 (MISO), PA7 (MOSI)
→ NSS(CS) 핀: PA4를 GPIO Output으로 수동 설정
```

### ADC 설정 예시 (로드셀)

```
왼쪽 → Analog → ADC1

IN0: "IN0 Single-ended" ← 채널 0 활성화
IN1: "IN1 Single-ended" ← 채널 1 활성화
IN2: "IN2 Single-ended" ← 채널 2 활성화
IN3: "IN3 Single-ended" ← 채널 3 활성화

Parameter Settings:
  Resolution:      16 Bits ★ (STM32H743은 16비트 ADC!)
  Conversion Mode: Scan (여러 채널 순차 읽기)

→ 핀 자동 배정: PA0, PA1, PA2, PA3

16비트 ADC:
  기존 Teensy: 12비트 = 4096 단계
  STM32H743:  16비트 = 65536 단계 ← 16배 정밀!
  → INA828 로드셀 값을 더 정확하게 읽을 수 있음
```

### 타이머 설정 (500Hz 제어 루프)

```
왼쪽 → Timers → TIM6

Mode: "Activated"

Parameter Settings:
  Prescaler:   24000 - 1 = 23999
  Counter Period: 40 - 1 = 39

  계산:
  타이머 클럭: 240MHz (APB1)
  240MHz / 24000 = 10kHz
  10kHz / 40 = 250Hz...

  수정:
  240MHz / 24000 = 10kHz
  10kHz / 20 = 500Hz ✅
  → Counter Period: 20 - 1 = 19

NVIC Settings:
  → TIM6 global interrupt: Enable ✅
  → 타이머가 20번 카운트할 때마다 인터럽트 발생 = 500Hz!

인터럽트 =
  "이 일 끝나면 알려줘!" 하는 것
  → 500Hz마다 자동으로 콜백 함수 호출
  → 그 함수 안에서 모터 제어 실행
```

---

## 5. 코드 자동 생성

### Generate Code

```
① 상단 메뉴: Project → Generate Code
   또는 톱니바퀴 아이콘 클릭

② CubeMX가 자동으로 생성하는 파일들:

AR_Walker_STM32/
├── Core/
│   ├── Inc/                    ← 헤더 파일
│   │   ├── main.h
│   │   ├── stm32h7xx_hal_conf.h  ← HAL 설정
│   │   └── stm32h7xx_it.h       ← 인터럽트 핸들러
│   │
│   └── Src/                    ← 소스 파일
│       ├── main.c              ← 메인! 여기에 코드 작성 ★
│       ├── stm32h7xx_hal_msp.c ← 핀 초기화 (자동 생성)
│       ├── stm32h7xx_it.c      ← 인터럽트 핸들러
│       └── system_stm32h7xx.c  ← 시스템 초기화
│
├── Drivers/
│   ├── CMSIS/                  ← ARM 공통 라이브러리
│   └── STM32H7xx_HAL_Driver/   ← ST HAL 라이브러리
│       ├── Inc/
│       └── Src/
│           ├── stm32h7xx_hal_fdcan.c  ← CAN FD 드라이버
│           ├── stm32h7xx_hal_uart.c   ← UART 드라이버
│           ├── stm32h7xx_hal_spi.c    ← SPI 드라이버
│           ├── stm32h7xx_hal_adc.c    ← ADC 드라이버
│           └── ...
│
└── AR_Walker_STM32.ioc         ← CubeMX 설정 파일 (이걸 열면 GUI로 돌아감)
```

### main.c 구조

```c
/* main.c — 자동 생성된 구조 */

#include "main.h"

// CubeMX가 만든 핸들 (각 주변장치를 제어하는 구조체)
FDCAN_HandleTypeDef hfdcan1;   // CAN FD 채널 1 (Elmo 모터 1)
FDCAN_HandleTypeDef hfdcan2;   // CAN FD 채널 2 (Elmo 모터 2)
UART_HandleTypeDef huart1;     // UART 1 (Jetson)
UART_HandleTypeDef huart2;     // UART 2 (EBIMU)
UART_HandleTypeDef huart3;     // UART 3 (BMS)
SPI_HandleTypeDef hspi1;       // SPI 1 (ESP32)
ADC_HandleTypeDef hadc1;       // ADC 1 (로드셀)
TIM_HandleTypeDef htim6;       // 타이머 6 (500Hz 루프)

int main(void)
{
    HAL_Init();                // HAL 라이브러리 초기화
    SystemClock_Config();      // 클럭 설정 (480MHz)

    // 각 주변장치 초기화 (CubeMX가 자동 생성)
    MX_FDCAN1_Init();
    MX_FDCAN2_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_SPI1_Init();
    MX_ADC1_Init();
    MX_TIM6_Init();

    /* USER CODE BEGIN 2 */
    // ★ 여기에 우리 초기화 코드 작성!
    // 예: CAN FD 시작, 타이머 시작 등
    HAL_TIM_Base_Start_IT(&htim6);  // 500Hz 타이머 인터럽트 시작
    /* USER CODE END 2 */

    while (1)
    {
        /* USER CODE BEGIN 3 */
        // ★ 메인 루프 (급하지 않은 작업)
        // 예: BLE 데이터 처리, GUI 업데이트 등
        /* USER CODE END 3 */
    }
}

// ★ 500Hz 타이머 콜백 — 여기가 핵심 제어 루프!
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6)
    {
        // 500Hz마다 실행되는 코드
        // 1. IMU 데이터 읽기
        // 2. 로드셀 읽기
        // 3. 제어 계산
        // 4. 모터 명령 전송
    }
}
```

### USER CODE BEGIN / END 이 뭐야?

```
CubeMX에서 설정을 바꾸고 다시 Generate Code를 하면:
  → 자동 생성 코드는 덮어써짐
  → BUT "USER CODE" 블록 안의 코드는 보존됨!

/* USER CODE BEGIN 2 */
// 여기 안에 쓴 코드는 안전! ✅
/* USER CODE END 2 */

// 여기에 쓰면 다음 Generate 때 날아감! ❌

→ 반드시 USER CODE 블록 안에 코드를 써야 함!
```

---

## 6. 핀맵 정리 — AR Walker 전체

```
STM32H743VIT6 핀 배정 요약:

기능          │ 핀         │ 용도
──────────────┼───────────┼──────────────────
FDCAN1_TX     │ PD1       │ Elmo 모터 1
FDCAN1_RX     │ PD0       │ Elmo 모터 1
FDCAN2_TX     │ PB13      │ Elmo 모터 2
FDCAN2_RX     │ PB12      │ Elmo 모터 2
USART1_TX     │ PA9       │ Jetson 통신
USART1_RX     │ PA10      │ Jetson 통신
USART2_TX     │ PD5       │ EBIMU RF
USART2_RX     │ PD6       │ EBIMU RF
USART3_TX     │ PB10      │ BMS 통신
USART3_RX     │ PB11      │ BMS 통신
SPI1_SCK      │ PA5       │ ESP32 클럭
SPI1_MISO     │ PA6       │ ESP32 → STM32
SPI1_MOSI     │ PA7       │ STM32 → ESP32
SPI1_CS       │ PA4       │ ESP32 칩 선택
ADC1_IN0      │ PA0       │ 로드셀 1
ADC1_IN1      │ PA1       │ 로드셀 2
ADC1_IN2      │ PA2       │ 로드셀 3
ADC1_IN3      │ PA3       │ 로드셀 4
TIM6          │ (내부)     │ 500Hz 인터럽트
LED           │ PB0       │ 상태 표시
HSE_IN        │ PH0       │ Crystal 입력
HSE_OUT       │ PH1       │ Crystal 출력
SWDIO         │ PA13      │ ST-Link 디버깅
SWCLK         │ PA14      │ ST-Link 디버깅

사용 핀: 약 24개 / 100개
→ 여유 충분!
```

---

## 7. 다음에 할 것 (보드 도착 후)

```
보드 없이 여기까지 완료하면:
  ✅ CubeIDE 설치됨
  ✅ 프로젝트 생성됨
  ✅ 클럭 480MHz 설정됨
  ✅ 모든 핀 배정됨
  ✅ 코드 뼈대 자동 생성됨
  ✅ 컴파일까지 가능! (에러 없는지 확인)

보드 오면:
  ⬜ USB 연결 → 코드 업로드 → LED 깜빡이기 (Hello World!)
  ⬜ UART printf 테스트 → PC에서 시리얼 모니터로 확인
  ⬜ CAN FD 루프백 테스트 → 자기 자신에게 CAN 메시지 보내기
  ⬜ ADC 읽기 테스트 → 가변저항 연결해서 값 변화 확인
```

---

## 연습 문제

### 문제 1: 클럭 계산
> Crystal이 25MHz이고, PLL로 480MHz를 만들려면 몇 배를 곱해야 하나?

<details>
<summary>정답 보기</summary>

```
480MHz / 25MHz = 19.2배

BUT PLL은 정수 분주/체배 조합으로 만듦:
  25MHz / 5 = 5MHz (분주)
  5MHz × 96 = 480MHz (체배)
  → PLLM = 5, PLLN = 96

CubeMX가 자동으로 계산해주니까 직접 할 필요 없음!
```
</details>

### 문제 2: 타이머 주기 계산
> 타이머 클럭이 240MHz이고, 500Hz 인터럽트를 만들고 싶다.
> Prescaler = 24000이면 Counter Period는?

<details>
<summary>정답 보기</summary>

```
타이머 클럭 / Prescaler = 240MHz / 24000 = 10kHz
10kHz / 500Hz = 20
Counter Period = 20 - 1 = 19 (0부터 세니까 -1)

→ Prescaler: 23999, Counter Period: 19
```
</details>

### 문제 3: ADC 정밀도
> 12비트 ADC와 16비트 ADC에서 3.3V를 측정할 때,
> 각각 1 단계(LSB)는 몇 mV인가?

<details>
<summary>정답 보기</summary>

```
12비트: 3.3V / 4096 = 0.806mV/step
16비트: 3.3V / 65536 = 0.050mV/step ← 16배 정밀!

→ STM32H743의 16비트 ADC로 로드셀을 더 정밀하게 읽을 수 있음
```
</details>

---

## 다음 단계
→ [Step 7: UART 통신 구현](step07_uart.md) (보드 도착 후)
