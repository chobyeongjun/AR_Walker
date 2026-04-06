---
title: "STM32 로봇 보드 개발 완전 가이드"
description: "STM32H743VITx 기반 로봇 보드 개발. ARM Cortex-M7 아키텍처, 핀 시스템, 클럭, GPIO, FDCAN/UART/SPI/ADC/PWM, CubeMX 설정, Teensy→STM32 핀 매핑, 보드 브링업까지."
date: "2026-04-06"
tags:
  - "STM32"
  - "ARM Cortex-M7"
  - "Robot"
  - "CAN Bus"
  - "HAL"
  - "CubeMX"
---


![STM32 로봇 보드 개발 히어로](/images/study/stm32/stm32-hero.png)
*STM32 + 로봇 보드 개발 — TechBlips*

---

## 1. STM32 아키텍처 기초

![ARM Cortex-M7 아키텍처](/images/study/stm32/cortex-m7-block.png)
*Cortex-M7 코어 블록 다이어그램*


### 1.1 ARM Cortex-M7 코어

STM32H743VITx는 ARM Cortex-M7 코어를 탑재한 고성능 마이크로컨트롤러이다.

**코어 특성:**

| 항목 | 사양 |
|------|------|
| 아키텍처 | ARMv7E-M |
| 파이프라인 | 6단계 슈퍼스칼라 (듀얼 이슈) |
| FPU | 단정밀도(SP) + 배정밀도(DP) 부동소수점 연산 |
| DSP | 단일 사이클 MAC, SIMD 명령어 |
| I-Cache | 16 KB (명령어 캐시) |
| D-Cache | 16 KB (데이터 캐시) |
| MPU | 16 리전 메모리 보호 유닛 |
| 최대 클럭 | 480 MHz |

**왜 로봇 보드에 Cortex-M7인가?**
- 500Hz 제어 루프를 안정적으로 구동 (현재 Teensy 4.1과 동일 코어)
- FPU로 PID 연산, 토크 계산 등 실수 연산을 하드웨어로 처리
- DSP 명령어로 센서 데이터 필터링 (IMU, 로드셀) 가속
- 캐시로 Flash에서 코드 실행 시에도 고속 동작 보장

### 1.2 STM32H743VITx 칩 스펙 요약

| 항목 | 사양 | 비고 |
|------|------|------|
| **패키지** | LQFP-100 | 100핀, 14x14mm |
| **Flash** | 2 MB | 듀얼 뱅크 |
| **RAM 총합** | 1 MB | 아래 상세 |
| **DTCM** | 128 KB | 가장 빠른 RAM (0사이클 대기) |
| **AXI SRAM** | 512 KB | 범용 대용량 |
| **SRAM1** | 128 KB | D2 도메인 |
| **SRAM2** | 128 KB | D2 도메인 |
| **SRAM3** | 32 KB | D2 도메인 |
| **SRAM4** | 64 KB | D3 도메인 |
| **Backup SRAM** | 4 KB | 배터리 백업 |
| **GPIO** | 최대 82개 | LQFP-100에서 사용 가능 |
| **ADC** | 3개 (ADC1/2/3) | 16비트, 3.6 MSPS |
| **FDCAN** | 2개 | CAN FD 지원 |
| **UART/USART** | 8개 | USART1-3,6 + UART4,5,7,8 |
| **SPI** | 6개 | SPI1-6 |
| **I2C** | 4개 | I2C1-4 |
| **Timer** | 다수 | TIM1-17 (Advanced, GP, Basic) |
| **동작 전압** | 1.62V ~ 3.6V | 일반적으로 3.3V 사용 |

### 1.3 메모리 맵

STM32H7은 전원 도메인(Power Domain)별로 메모리와 페리페럴이 구분된다.

![메모리 맵](/images/study/stm32/memory-map.png)
*STM32H743 메모리 맵 — 주소별 영역 구분*

**로봇 보드에서의 메모리 활용 전략:**
- **DTCM (128KB)**: 제어 루프 변수, PID 파라미터, 모터 명령 버퍼 → 가장 빠른 접근
- **AXI SRAM (512KB)**: ExoData 구조체, 센서 데이터 배열, 설정 파일 파싱 버퍼
- **SRAM1/2 (256KB)**: DMA 전송 버퍼 (ADC, UART, SPI) → D2 도메인의 DMA가 직접 접근
- **SRAM4 (64KB)**: 저전력 모드에서도 유지해야 할 데이터

### 1.4 버스 아키텍처

STM32H7의 버스는 3개의 전원 도메인(D1, D2, D3)으로 나뉜다:

![버스 도메인](/images/study/stm32/bus-domains.png)
*D1/D2/D3 전원 도메인과 버스 아키텍처*

**핵심 포인트:**
- GPIO는 AHB4 (D3 도메인)에 연결되어 있어 모든 도메인에서 접근 가능
- FDCAN1/2는 APB1 (D2 도메인)에 있으므로 DMA1/2와 함께 사용 시 SRAM1/2에 버퍼 배치
- ADC1/2는 APB2에, ADC3는 AHB4에 있어 서로 다른 도메인 → DMA 버퍼 위치 주의

---

## 2. 핀 시스템 완전 정복

![LQFP-100 핀아웃](/images/study/stm32/lqfp100-pinout.png)
*STM32H743VITx LQFP-100 핀아웃 다이어그램 — Gemini로 생성 필요*


### 2.1 물리적 핀 vs GPIO 포트

STM32의 핀 시스템을 이해하려면 두 가지 "핀 번호"의 차이를 알아야 한다:

1. **물리적 핀 번호**: IC 패키지의 다리 번호 (LQFP-100: 1번~100번)
2. **GPIO 포트 이름**: PA0, PB3, PC13 등 소프트웨어에서 사용하는 이름

**GPIO 포트 명명 규칙:**

```
P + [포트 문자] + [핀 번호]
│    │              │
│    A~K            0~15
│    (포트 그룹)    (그룹 내 핀 번호)
GPIO
```

예시:
- `PA0` = Port A, Pin 0
- `PB7` = Port B, Pin 7
- `PD1` = Port D, Pin 1

STM32H743VITx (LQFP-100)에서 사용 가능한 GPIO 포트:

| 포트 | 사용 가능한 핀 | 비고 |
|------|---------------|------|
| GPIOA | PA0~PA15 | 16핀 전부 사용 가능 |
| GPIOB | PB0~PB15 | 16핀 전부 사용 가능 |
| GPIOC | PC0~PC13 | 14핀 (PC14/15는 OSC32) |
| GPIOD | PD0~PD15 | 16핀 전부 사용 가능 |
| GPIOE | PE0~PE15 | 16핀 전부 사용 가능 |

> ⚠️ **주의**: LQFP-100 패키지에서는 GPIOF~GPIOK는 사용 불가 (핀이 없음).
> 144핀 이상의 패키지에서만 사용 가능.

### 2.2 LQFP-100 핀 번호와 GPIO 매핑

LQFP-100 패키지에서 물리적 핀 번호와 GPIO 이름의 매핑 (STM32H743VITx 기준):

```
                      핀 76~100 (상단)
                ┌──────────────────────┐
                │  76 77 78 ... 99 100 │
        핀      │                      │  핀
       51~75    │                      │  1~25
       (우측)   │    STM32H743VITx     │  (좌측)
                │      LQFP-100        │
                │                      │
                │  51 50 49 ... 27 26  │
                └──────────────────────┘
                      핀 26~50 (하단)
```

**주요 핀 매핑 (자주 사용하는 것들):**

| 물리 핀 | GPIO | 주요 기능 | AF 예시 |
|---------|------|----------|---------|
| 14 | PA0 | ADC1_IN0, TIM2_CH1 | AF1(TIM2), AF-(Analog) |
| 15 | PA1 | ADC1_IN1, TIM2_CH2 | AF1(TIM2), AF-(Analog) |
| 16 | PA2 | ADC1_IN2, USART2_TX | AF7(USART2), AF-(Analog) |
| 17 | PA3 | ADC1_IN3, USART2_RX | AF7(USART2), AF-(Analog) |
| 20 | PA4 | ADC1_IN4, SPI1_NSS, DAC1_OUT1 | AF5(SPI1) |
| 21 | PA5 | ADC1_IN5, SPI1_SCK, DAC1_OUT2 | AF5(SPI1) |
| 22 | PA6 | ADC1_IN6, SPI1_MISO, TIM3_CH1 | AF5(SPI1) |
| 23 | PA7 | ADC1_IN7, SPI1_MOSI, TIM3_CH2 | AF5(SPI1) |
| 68 | PA8 | TIM1_CH1, MCO1 | AF1(TIM1), AF0(MCO1) |
| 69 | PA9 | USART1_TX, TIM1_CH2 | AF7(USART1) |
| 70 | PA10 | USART1_RX, TIM1_CH3 | AF7(USART1) |
| 71 | PA11 | USB_OTG_FS_DM, FDCAN1_RX | AF9(FDCAN1) |
| 72 | PA12 | USB_OTG_FS_DP, FDCAN1_TX | AF9(FDCAN1) |
| 76 | PA13 | **SWDIO** (디버거) | 디버깅 전용 — 변경 금지! |
| 77 | PA14 | **SWCLK** (디버거) | 디버깅 전용 — 변경 금지! |
| 78 | PA15 | SPI3_NSS, TIM2_CH1 | AF6(SPI3) |
| 35 | PB0 | ADC1_IN8, TIM3_CH3 | AF2(TIM3) |
| 36 | PB1 | ADC1_IN9, TIM3_CH4 | AF2(TIM3) |
| 37 | PB2 | QUADSPI_CLK | AF9(QUADSPI) |
| 89 | PB3 | SPI3_SCK, TIM2_CH2 | AF6(SPI3) |
| 90 | PB4 | SPI3_MISO, TIM3_CH1 | AF6(SPI3) |
| 91 | PB5 | SPI3_MOSI, FDCAN2_RX | AF6(SPI3), AF9(FDCAN2) |
| 92 | PB6 | USART1_TX, FDCAN2_TX, I2C1_SCL | AF7(USART1), AF9(FDCAN2) |
| 93 | PB7 | USART1_RX, I2C1_SDA | AF7(USART1), AF4(I2C1) |
| 95 | PB8 | FDCAN1_RX, I2C1_SCL, TIM4_CH3 | AF9(FDCAN1), AF4(I2C1) |
| 96 | PB9 | FDCAN1_TX, I2C1_SDA, TIM4_CH4 | AF9(FDCAN1), AF4(I2C1) |
| 47 | PB10 | USART3_TX, I2C2_SCL, TIM2_CH3 | AF7(USART3) |
| 48 | PB11 | USART3_RX, I2C2_SDA, TIM2_CH4 | AF7(USART3) |
| 51 | PB12 | SPI2_NSS, USART3_CK | AF5(SPI2) |
| 52 | PB13 | SPI2_SCK, FDCAN2_TX | AF5(SPI2), AF9(FDCAN2) |
| 53 | PB14 | SPI2_MISO, USART3_RTS | AF5(SPI2) |
| 54 | PB15 | SPI2_MOSI, TIM12_CH2 | AF5(SPI2) |
| 8 | PC0 | ADC1_IN10 | AF-(Analog) |
| 9 | PC1 | ADC1_IN11 | AF-(Analog) |
| 10 | PC2 | ADC1_IN12, SPI2_MISO | AF5(SPI2) |
| 11 | PC3 | ADC1_IN13, SPI2_MOSI | AF5(SPI2) |
| 33 | PC4 | ADC1_IN14 | AF-(Analog) |
| 34 | PC5 | ADC1_IN15 | AF-(Analog) |
| 63 | PC6 | USART6_TX, TIM8_CH1 | AF7(USART6) |
| 64 | PC7 | USART6_RX, TIM8_CH2 | AF7(USART6) |
| 65 | PC8 | TIM8_CH3, SDMMC1_D0 | AF3(TIM8) |
| 66 | PC9 | TIM8_CH4, SDMMC1_D1 | AF3(TIM8) |
| 67 | PC10 | USART3_TX, SPI3_SCK | AF7(USART3) |
| 78 | PC11 | USART3_RX, SPI3_MISO | AF7(USART3) |
| 80 | PC12 | SPI3_MOSI, UART5_TX | AF6(SPI3) |
| 7 | PC13 | RTC_TAMP, WKUP | 범용 GPIO |
| 82 | PD0 | FDCAN1_RX, FMC_D2 | AF9(FDCAN1) |
| 83 | PD1 | FDCAN1_TX, FMC_D3 | AF9(FDCAN1) |
| 84 | PD2 | UART5_RX, TIM3_ETR | AF8(UART5) |
| 85 | PD3 | USART2_CTS, FMC_CLK | AF7(USART2) |
| 86 | PD4 | USART2_RTS, FMC_NOE | AF7(USART2) |
| 87 | PD5 | USART2_TX, FMC_NWE | AF7(USART2) |
| 88 | PD6 | USART2_RX, FMC_NWAIT | AF7(USART2) |
| 55 | PD8 | USART3_TX | AF7(USART3) |
| 56 | PD9 | USART3_RX | AF7(USART3) |
| 100 | PE0 | UART8_RX, TIM4_ETR | AF8(UART8) |
| 1 | PE1 | UART8_TX | AF8(UART8) |
| 2 | PE2 | SPI4_SCK, UART8_TX | AF5(SPI4) |
| 98 | PE9 | TIM1_CH1, FMC_D6 | **AF1(TIM1)** — PWM용 |
| 99 | PE11 | TIM1_CH2, FMC_D8 | **AF1(TIM1)** — PWM용 |

### 2.3 핀 기능 분류

LQFP-100의 100개 핀은 크게 4종류로 나뉜다:

| 분류 | 핀 | 개수 | 설명 |
|------|-----|------|------|
| **전원** | VDD, VSS, VDDA, VSSA, VREF+, VCAP | ~18개 | 전원 공급 (3.3V, GND) |
| **클럭** | OSC_IN, OSC_OUT (PH0/PH1) | 2개 | 외부 크리스탈 연결 |
| **리셋** | NRST | 1개 | 시스템 리셋 (액티브 로우) |
| **디버그** | PA13 (SWDIO), PA14 (SWCLK) | 2개 | SWD 디버거 전용 |
| **부트** | BOOT0 | 1개 | 부트 모드 선택 |
| **GPIO** | PA0~PE15 | ~76개 | 범용 입출력 + AF |

> 🔴 **절대 규칙**: PA13/PA14는 SWD 디버거 핀이다. 이 핀을 다른 용도로 사용하면 디버깅/프로그래밍이 불가능해진다. 반드시 SWD 전용으로 남겨둘 것!

### 2.4 Alternate Function (AF) 시스템

STM32에서 가장 중요한 개념 중 하나. 하나의 GPIO 핀이 최대 16가지 다른 기능을 수행할 수 있다.

**AF 번호 체계 (AF0 ~ AF15):**

| AF 번호 | 주요 기능 |
|---------|----------|
| AF0 | SYS (MCO, SWD, TRACE) |
| AF1 | TIM1, TIM2 |
| AF2 | TIM3, TIM4, TIM5 |
| AF3 | TIM8, TIM15-17 |
| AF4 | I2C1-4 |
| AF5 | SPI1-6 |
| AF6 | SPI2/3, SAI1/2 |
| AF7 | USART1-3, USART6 |
| AF8 | UART4/5, UART7/8, LPUART1 |
| AF9 | FDCAN1/2, QUADSPI, TIM12-14 |
| AF10 | USB OTG, SAI2 |
| AF11 | ETH, UART7 |
| AF12 | FMC, SDMMC, MDIOS |
| AF13 | DCMI, COMP |
| AF14 | LTDC, UART5 |
| AF15 | EVENTOUT |

![AF 멀티플렉서](/images/study/stm32/af-mux.png)
*AF0~AF15 멀티플렉서 — 하나의 핀에 16가지 기능 선택*

**AF 사용 예시 — PA7 핀:**

PA7에는 다음 기능들이 할당 가능:
- AF0: MCO (마이크로컨트롤러 클럭 출력)
- AF2: TIM3_CH2 (타이머3 채널2 = PWM 출력)
- AF5: SPI1_MOSI (SPI1 마스터 출력)
- Analog: ADC1_IN7 (아날로그 입력)

**한 번에 하나의 AF만 선택 가능!** 만약 PA7을 SPI1_MOSI로 사용하면 이 핀에서 ADC나 TIM3는 동시에 사용할 수 없다.

**AF 충돌 예방법:**
1. 필요한 페리페럴 목록 작성 (CAN, UART, SPI, ADC, PWM 등)
2. 각 페리페럴의 핀 옵션 확인 (데이터시트 Table 9-12)
3. AF가 겹치지 않도록 핀 배치 → CubeMX가 자동으로 충돌 검사해줌

### 2.5 핀 설정 레지스터

각 GPIO 핀은 다음 레지스터들로 제어된다:

```
레지스터          비트 수/핀    기능
──────────────────────────────────────────────
GPIOx_MODER      2비트/핀     모드 선택
                              00 = Input
                              01 = Output
                              10 = Alternate Function
                              11 = Analog

GPIOx_OTYPER     1비트/핀     출력 타입
                              0 = Push-Pull
                              1 = Open-Drain

GPIOx_OSPEEDR    2비트/핀     출력 속도
                              00 = Low (최대 12MHz)
                              01 = Medium (최대 60MHz)
                              10 = High (최대 85MHz)
                              11 = Very High (최대 100MHz)

GPIOx_PUPDR      2비트/핀     풀업/풀다운
                              00 = No Pull
                              01 = Pull-Up
                              10 = Pull-Down
                              11 = Reserved

GPIOx_AFRL       4비트/핀     AF 선택 (핀 0~7)
GPIOx_AFRH       4비트/핀     AF 선택 (핀 8~15)
                              0000 = AF0
                              0001 = AF1
                              ...
                              1111 = AF15
```

**레지스터 직접 조작 예시 (PA7을 SPI1_MOSI AF5로 설정):**

```c
// 1. GPIOA 클럭 활성화
RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;

// 2. PA7을 Alternate Function 모드로 설정
GPIOA->MODER &= ~(0x3 << (7 * 2));     // 비트 클리어
GPIOA->MODER |=  (0x2 << (7 * 2));     // 10 = AF 모드

// 3. AF5 (SPI1) 선택 — PA7은 핀 0~7이므로 AFRL 사용
GPIOA->AFR[0] &= ~(0xF << (7 * 4));    // 비트 클리어
GPIOA->AFR[0] |=  (0x5 << (7 * 4));    // AF5 = SPI1

// 4. Push-Pull, Very High Speed
GPIOA->OTYPER &= ~(1 << 7);            // Push-Pull
GPIOA->OSPEEDR |= (0x3 << (7 * 2));    // Very High Speed
```

> **실무에서는 HAL 라이브러리를 사용하므로 레지스터를 직접 다룰 일은 드물다.**
> 하지만 레지스터 구조를 이해하면 디버깅 시 레지스터 값을 읽고 문제를 진단할 수 있다.

### 2.6 데이터시트에서 핀 정보 찾는 법

STM32H743VITx의 핀 정보를 찾으려면 두 문서가 필요하다:

| 문서 | 내용 | 찾을 정보 |
|------|------|----------|
| **Datasheet** (DS12110) | 핀아웃, 전기적 특성, 패키지 | 물리적 핀 번호, AF 테이블, 전압/전류 스펙 |
| **Reference Manual** (RM0433) | 레지스터, 페리페럴 동작 | GPIO 레지스터, 타이머 설정, CAN 프로토콜 등 |

**데이터시트에서 핵심적으로 봐야 할 섹션:**
- **Table 9~12**: "Alternate function mapping" — 각 핀의 AF0~AF15 매핑 전체 표
- **Figure 1**: LQFP-100 핀아웃 다이어그램
- **Table 7**: "Pin definitions" — 각 핀의 전원/IO 타입, 5V 톨러런트 여부

> 💡 **팁**: CubeMX를 사용하면 이 테이블을 GUI로 편하게 볼 수 있지만,
> 데이터시트를 직접 읽는 습관을 들이면 하드웨어 설계 시 큰 도움이 된다.

---

## 3. 클럭 시스템

![클럭 트리](/images/study/stm32/clock-tree.png)
*HSE → PLL → SYSCLK 클럭 트리 — Gemini로 생성 필요*


### 3.1 클럭 소스

STM32H743에는 4가지 클럭 소스가 있다:

| 클럭 소스 | 주파수 | 정밀도 | 용도 |
|-----------|--------|--------|------|
| **HSE** (High-Speed External) | 4~50 MHz (보통 8 또는 25 MHz) | 높음 (크리스탈) | PLL 입력 → SYSCLK |
| **HSI** (High-Speed Internal) | 64 MHz 고정 | 보통 (RC 발진기) | 리셋 후 기본 클럭 |
| **LSE** (Low-Speed External) | 32.768 kHz | 높음 | RTC, 저전력 모드 |
| **LSI** (Low-Speed Internal) | 32 kHz | 낮음 | 독립 워치독 (IWDG) |

**로봇 보드에서의 선택:**
- **HSE 8MHz 크리스탈 사용 권장** → PLL을 통해 480MHz SYSCLK 생성
- LSE 32.768kHz는 선택사항 (RTC 필요 시)
- HSI는 비상용 (크리스탈 고장 시 자동 전환 가능)

### 3.2 PLL 설정: 8MHz → 480MHz 달성 경로

STM32H7에는 3개의 PLL이 있다. 메인 PLL1으로 SYSCLK을 생성한다:

```
HSE (8 MHz)
    │
    ▼
  DIVM1 = /1        ← PLL 입력 분주기
    │
    ▼
  8 MHz (PLL 입력)  ← 반드시 1~16 MHz 범위
    │
    ▼
  DIVN1 = x120      ← VCO 체배기 (곱하기)
    │
    ▼
  960 MHz (VCO)     ← VCO 범위: 192~960 MHz
    │
    ├─ DIVP1 = /2 ──→ 480 MHz ──→ SYSCLK (시스템 클럭)
    │
    ├─ DIVQ1 = /4 ──→ 240 MHz ──→ 일부 페리페럴 (FDCAN 등)
    │
    └─ DIVR1 = /2 ──→ 480 MHz ──→ (사용하지 않으면 비활성화)
```

**CubeMX에서의 설정값:**

| 파라미터 | 값 | 의미 |
|---------|-----|------|
| PLL Source | HSE | 외부 크리스탈 사용 |
| DIVM1 | 1 | 8MHz / 1 = 8MHz |
| DIVN1 | 120 | 8MHz x 120 = 960MHz (VCO) |
| DIVP1 | 2 | 960MHz / 2 = **480MHz** (SYSCLK) |
| DIVQ1 | 4 | 960MHz / 4 = 240MHz |
| DIVR1 | 2 | 960MHz / 2 = 480MHz (비활성화 가능) |

### 3.3 클럭 트리: SYSCLK에서 각 버스로 분배

```
SYSCLK (480 MHz)
    │
    ▼
  D1CPRE = /1
    │
    ▼
  CPU 클럭 = 480 MHz
    │
    ├── HPRE = /2 ──→ AHB 클럭 = 240 MHz
    │                    │
    │                    ├── D1PPRE = /2 ──→ APB3 = 120 MHz (타이머는 x2 = 240 MHz)
    │                    │
    │                    ├── D2PPRE1 = /2 ──→ APB1 = 120 MHz (타이머는 x2 = 240 MHz)
    │                    │                      └── TIM2-7, TIM12-14
    │                    │                      └── USART2/3, UART4/5/7/8
    │                    │                      └── SPI2/3, I2C1-3
    │                    │                      └── **FDCAN1/2**
    │                    │
    │                    ├── D2PPRE2 = /2 ──→ APB2 = 120 MHz (타이머는 x2 = 240 MHz)
    │                    │                      └── TIM1, TIM8, TIM15-17
    │                    │                      └── USART1/6
    │                    │                      └── SPI1/4/5
    │                    │                      └── ADC1/2
    │                    │
    │                    └── D3PPRE = /2 ──→ APB4 = 120 MHz
    │                                        └── I2C4, SPI6
    │                                        └── EXTI, RTC
    │
    └── SysTick = 480 MHz (기본) 또는 AHB/8
```

**핵심 정리:**

| 버스 | 주파수 | 타이머 클럭 | 연결된 주요 페리페럴 |
|------|--------|------------|---------------------|
| CPU | 480 MHz | — | Cortex-M7 코어 |
| AHB | 240 MHz | — | DMA, GPIO, Flash |
| APB1 | 120 MHz | 240 MHz | FDCAN, UART4/5/7/8, SPI2/3, TIM2-7 |
| APB2 | 120 MHz | 240 MHz | USART1/6, SPI1, ADC1/2, TIM1/8 |
| APB3 | 120 MHz | — | LTDC |
| APB4 | 120 MHz | — | I2C4, SPI6 |

> 🔴 **중요**: APB 버스에 연결된 **타이머**는 APB 분주 비율이 1이 아닌 경우 자동으로 **x2** 된다.
> APB1 = 120MHz이고 분주비 /2이므로, TIM2~7의 실제 클럭은 240MHz이다.

### 3.4 CubeMX 클럭 설정 실전

![CubeMX 클럭 설정](/images/study/stm32/cubemx-clock.png)
*CubeMX Clock Configuration 탭 설정 화면*

CubeMX의 Clock Configuration 탭에서:

1. **좌측**: HSE → PLL Source Mux에서 "HSE" 선택
2. **가운데**: PLL1 파라미터 입력 (DIVM=1, N=120, P=2, Q=4)
3. **System Clock Mux**: "PLLCLK" 선택
4. **우측**: 각 버스 분주기가 자동 설정됨
5. **확인**: "Resolve Clock Issues" 버튼으로 문제 없는지 확인

> CubeMX가 빨간색으로 표시하면 주파수가 최대값을 초과한 것이다.
> 이 경우 분주기를 조절하여 각 버스의 최대 주파수 이하로 맞춘다.

---

## 4. GPIO 설정 상세

![GPIO 4가지 모드](/images/study/stm32/gpio-modes.png)
*GPIO Input/Output/AF/Analog 모드 비교 — Gemini로 생성 필요*


### 4.1 GPIO 4가지 모드

모든 GPIO 핀은 4가지 모드 중 하나로 설정된다:

| 모드 | MODER 값 | 설명 | 사용 예시 |
|------|---------|------|----------|
| **Input** | 00 | 외부 신호 읽기 | 버튼, 모터 에러 핀, 인터럽트 입력 |
| **Output** | 01 | 신호 내보내기 | LED, 모터 Enable, Motor Stop |
| **Alternate Function** | 10 | 페리페럴에 핀 연결 | UART TX/RX, SPI, CAN, PWM |
| **Analog** | 11 | 아날로그 입출력 | ADC 입력 (토크센서, 각도센서), DAC |

### 4.2 Output Type: Push-Pull vs Open-Drain

![Push-Pull vs Open-Drain](/images/study/stm32/push-pull-od.png)
*Push-Pull과 Open-Drain 출력 비교*

Output 또는 AF 모드에서 출력 타입을 선택한다:

**Push-Pull (PP):**
```
VDD ─── [P-FET] ─┬── 핀 출력
                  │
GND ─── [N-FET] ─┘

출력 HIGH → P-FET ON, N-FET OFF → VDD 출력 (3.3V)
출력 LOW  → P-FET OFF, N-FET ON → GND 출력 (0V)
```
- 능동적으로 HIGH/LOW 모두 구동
- **대부분의 경우 Push-Pull 사용** (LED, SPI, UART TX, PWM 등)

**Open-Drain (OD):**
```
         ┌── 외부 풀업 저항 ── VDD (또는 5V!)
핀 출력 ──┤
         └── [N-FET] ── GND

출력 LOW  → N-FET ON → GND 출력
출력 HIGH → N-FET OFF → 풀업 저항에 의해 VDD로 올라감
```
- LOW만 능동 구동, HIGH는 외부 풀업에 의존
- **I2C 통신에 필수** (SDA/SCL)
- **레벨 시프팅**: 3.3V MCU에서 5V 장치와 통신 시 사용

### 4.3 Pull 설정

| 설정 | 효과 | 사용 시기 |
|------|------|----------|
| **No Pull** | 풀업/풀다운 없음 | AF 모드 (페리페럴이 제어), 외부 풀업/풀다운 있을 때 |
| **Pull-Up** | 내부 ~40kΩ 저항으로 VDD 연결 | 버튼 입력 (액티브 로우), UART RX 유휴 상태 |
| **Pull-Down** | 내부 ~40kΩ 저항으로 GND 연결 | 플로팅 방지, 기본값 LOW 필요 시 |

### 4.4 출력 속도

| 속도 | 최대 주파수 | 사용 시기 |
|------|-----------|----------|
| **Low** | ~12 MHz | GPIO 토글 (LED), 저속 신호 |
| **Medium** | ~60 MHz | UART, I2C |
| **High** | ~85 MHz | SPI, SDMMC |
| **Very High** | ~100 MHz | 고속 SPI, FMC |

> ⚠️ **규칙**: 필요한 최소 속도를 선택한다. 속도가 높을수록 EMI(전자파 간섭)가 증가하고 소비전류가 늘어난다.
> - LED, Enable 핀 → Low
> - CAN, UART → Medium
> - SPI → High 또는 Very High

### 4.5 HAL 라이브러리로 GPIO 설정

CubeMX가 자동 생성하는 코드의 구조:

```c
/* Core/Src/main.c — MX_GPIO_Init() 함수 내부 */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO 포트 클럭 활성화 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* === 예시 1: LED 출력 (PB0) === */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  // 초기값 LOW
    GPIO_InitStruct.Pin   = GPIO_PIN_0;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;    // Output Push-Pull
    GPIO_InitStruct.Pull  = GPIO_NOPULL;            // 풀업/풀다운 없음
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    // 저속 (LED니까)
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* === 예시 2: 버튼 입력 + 인터럽트 (PC13) === */
    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;   // 하강 에지 인터럽트
    GPIO_InitStruct.Pull  = GPIO_PULLUP;             // 내부 풀업
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* EXTI 인터럽트 활성화 */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /* === 예시 3: 모터 Enable 핀 (PD3) === */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin   = GPIO_PIN_3;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLDOWN;           // 기본 OFF (안전)
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}
```

### 4.6 GPIO 제어 함수

```c
/* 핀 출력 HIGH */
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

/* 핀 출력 LOW */
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

/* 핀 토글 */
HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

/* 핀 읽기 */
GPIO_PinState state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
if (state == GPIO_PIN_SET) {
    // HIGH 상태
}
```

### 4.7 실습: LED 깜빡이기 (첫 번째 테스트)

보드를 만들고 가장 먼저 해야 할 테스트:

```c
/* Core/Src/main.c */

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* USER CODE BEGIN 2 */
    // (추가 초기화 코드)
    /* USER CODE END 2 */

    while (1)
    {
        /* USER CODE BEGIN 3 */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // LED 토글
        HAL_Delay(500);                           // 500ms 대기
        /* USER CODE END 3 */
    }
}
```

> LED가 0.5초 간격으로 깜빡이면 다음을 확인한 것이다:
> 1. 전원이 정상 (VDD 3.3V)
> 2. 클럭이 정상 (HSE → PLL → SYSCLK)
> 3. GPIO가 정상 (출력 동작)
> 4. HAL 라이브러리가 정상 (HAL_Delay 동작 = SysTick 정상)
> 5. 플래시 프로그래밍이 정상 (코드가 실행됨)

---

## 5. 로봇에 필요한 핵심 페리페럴

![CAN 버스 토폴로지](/images/study/stm32/can-topology.png)
*CAN 버스 통신 구조 — Gemini로 생성 필요*


### 5.1 FDCAN (모터 CAN 통신)

AR_Walker의 T-Motor (AK60, AK70, AK80) 모터들은 CAN 버스로 통신한다.
STM32H7은 FDCAN (Flexible Data-rate CAN)을 지원하여 기존 CAN 2.0과 CAN FD 모두 사용 가능.

#### CAN vs CAN FD 비교

| 항목 | CAN 2.0 | CAN FD |
|------|---------|--------|
| 데이터 길이 | 최대 8 바이트 | 최대 64 바이트 |
| 비트레이트 | 최대 1 Mbps | Nominal 1M + Data 최대 8 Mbps |
| 현재 모터 사용 | **CAN 2.0** | (향후 확장 가능) |

> AR_Walker의 T-Motor는 CAN 2.0 (1Mbps, 8바이트)을 사용한다.
> FDCAN 페리페럴은 하위 호환성이 있어 CAN 2.0 모드로 동작 가능.

#### FDCAN 핀 옵션 (LQFP-100)

| 페리페럴 | TX 핀 옵션 | RX 핀 옵션 | AF |
|----------|-----------|-----------|-----|
| **FDCAN1** | PD1, PA12, PB9 | PD0, PA11, PB8 | **AF9** |
| **FDCAN2** | PB13, PB6 | PB5, PB12 | **AF9** |

**AR_Walker 권장 매핑:**
- FDCAN1: **PD1** (TX), **PD0** (RX) → AF9
- 이유: Port D에 배치하면 Port A의 ADC/SPI 핀과 충돌 없음

#### CubeMX 설정

1. Connectivity → FDCAN1 활성화
2. Parameter Settings:
   - Frame Format: **Classic** (CAN 2.0 모드)
   - Mode: **Normal** (루프백은 테스트용)
   - Nominal Prescaler: **3**
   - Nominal Time Seg1: **13**
   - Nominal Time Seg2: **2**
   - Nominal Sync Jump Width: **1**
   - → Nominal Bit Rate = **APB1_CLK / (Prescaler * (1 + Seg1 + Seg2))**
   - → 120MHz / (3 * (1+13+2)) = 120/48 = **2.5Mbps**... → 조정 필요
   - Prescaler: **10**, Seg1: **5**, Seg2: **6** → 120/(10*(1+5+6)) = **1 Mbps**

#### HAL 코드 예제

```c
/* FDCAN 초기화 — CubeMX가 자동 생성 */
FDCAN_HandleTypeDef hfdcan1;

/* USER CODE BEGIN: 필터 설정 + 시작 */
void FDCAN1_Start(void)
{
    FDCAN_FilterTypeDef filter;
    filter.IdType       = FDCAN_STANDARD_ID;
    filter.FilterIndex  = 0;
    filter.FilterType   = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = 0x000;    // 모든 ID 수신
    filter.FilterID2    = 0x000;    // 마스크: 모든 비트 무시 (= 전부 수신)
    HAL_FDCAN_ConfigFilter(&hfdcan1, &filter);

    // FIFO0 수신 인터럽트 활성화
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    // CAN 시작
    HAL_FDCAN_Start(&hfdcan1);
}

/* CAN 메시지 송신 — 모터 명령 전송 */
void CAN_SendMotorCommand(uint16_t motor_id, float torque)
{
    FDCAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];

    tx_header.Identifier          = motor_id;       // 예: 0x01 (모터 ID)
    tx_header.IdType              = FDCAN_STANDARD_ID;
    tx_header.TxFrameType         = FDCAN_DATA_FRAME;
    tx_header.DataLength          = FDCAN_DLC_BYTES_8;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch       = FDCAN_BRS_OFF;  // CAN 2.0 모드
    tx_header.FDFormat            = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker       = 0;

    // 토크 값을 CAN 데이터로 인코딩 (모터 프로토콜에 따라)
    // T-Motor AK 시리즈의 CAN 프로토콜:
    // [pos(15:8)] [pos(7:0)] [vel(11:4)] [vel(3:0)|kp(11:8)]
    // [kp(7:0)] [kd(11:4)] [kd(3:0)|torque(11:8)] [torque(7:0)]
    encode_motor_command(tx_data, 0.0f, 0.0f, 0.0f, 0.0f, torque);

    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, tx_data);
}

/* CAN 수신 콜백 — 모터 응답 처리 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
    {
        uint16_t motor_id = rx_header.Identifier;
        // 모터 응답 파싱: 위치, 속도, 전류
        float position, velocity, current;
        decode_motor_response(rx_data, &position, &velocity, &current);

        // ExoData 구조체에 저장
        update_motor_data(motor_id, position, velocity, current);
    }
}
```

> 🔧 **하드웨어 참고**: CAN 버스에는 **CAN 트랜시버 IC** (예: MCP2562, SN65HVD230)가 필요하다.
> MCU의 FDCAN TX/RX 핀 → 트랜시버 → CAN_H/CAN_L 차동 신호 → 모터.
> 버스 양 끝에 **120Ω 종단 저항** 필수.

---

### 5.2 UART/USART (IMU 통신)

AR_Walker의 IMU는 UART 시리얼 통신으로 데이터를 전송한다.
현재 Teensy에서는 Serial4 (RX4 = pin 16)을 사용.

#### UART 핀 옵션 (LQFP-100에서 자주 사용)

| 페리페럴 | TX 핀 | RX 핀 | AF | 버스 |
|----------|-------|-------|-----|------|
| **USART1** | PA9, PB6 | PA10, PB7 | AF7 | APB2 |
| **USART2** | PA2, PD5 | PA3, PD6 | AF7 | APB1 |
| **USART3** | PB10, PC10, PD8 | PB11, PC11, PD9 | AF7 | APB1 |
| **USART6** | PC6 | PC7 | AF7 | APB2 |
| **UART4** | PA0, PC10 | PA1, PC11 | AF8 | APB1 |
| **UART5** | PC12 | PD2 | AF8/AF14 | APB1 |
| **UART7** | PE8 | PE7 | AF7 | APB1 |
| **UART8** | PE1 | PE0 | AF8 | APB1 |

**AR_Walker IMU 매핑 권장:**
- UART4: **PA1** (RX) → AF8 (TX 불필요, RX만 사용)
- 또는 USART3: **PD9** (RX), PD8 (TX) → AF7

> 💡 **USART vs UART 차이**: USART는 동기 모드(클럭 동기화) 지원, UART는 비동기만.
> IMU 통신은 비동기이므로 어느 것이든 가능.

#### CubeMX 설정

1. Connectivity → UART4 (또는 원하는 UART) 활성화
2. Mode: **Asynchronous**
3. Parameter Settings:
   - Baud Rate: **115200** (또는 IMU 스펙에 맞춤)
   - Word Length: **8 Bits**
   - Stop Bits: **1**
   - Parity: **None**
   - Over Sampling: **16**

#### HAL 코드 예제

```c
UART_HandleTypeDef huart4;

/* === DMA를 사용한 효율적 수신 (권장) === */

uint8_t imu_rx_buffer[24];  // IMU 패킷 크기에 맞춤

void IMU_StartReceive(void)
{
    // DMA로 순환 수신 시작 — CPU를 블로킹하지 않음
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, imu_rx_buffer, sizeof(imu_rx_buffer));
    __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);  // Half-Transfer 인터럽트 비활성화
}

/* DMA 수신 완료 또는 Idle Line 감지 시 콜백 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart4)
    {
        // IMU 데이터 파싱
        parse_imu_data(imu_rx_buffer, Size);

        // 다음 수신 재시작
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, imu_rx_buffer, sizeof(imu_rx_buffer));
        __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);
    }
}

/* === 간단한 폴링 방식 (디버깅용) === */
void IMU_ReadPolling(void)
{
    uint8_t byte;
    if (HAL_UART_Receive(&huart4, &byte, 1, 1) == HAL_OK)
    {
        // 바이트 단위 처리
        process_imu_byte(byte);
    }
}

/* === 디버그 UART 출력 (printf 리다이렉트) === */
// USART3를 디버그용으로 사용하는 경우:
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
// 이후 printf("Hello STM32!\n"); 로 시리얼 출력 가능
```

> 💡 **DMA 수신이 중요한 이유**: IMU가 500Hz로 데이터를 보내면, 폴링 방식은 제어 루프(500Hz)와 타이밍이 충돌할 수 있다.
> DMA를 사용하면 CPU 개입 없이 백그라운드로 수신되므로 제어 루프에 영향을 주지 않는다.

---

### 5.3 SPI (MCU 간 통신)

현재 AR_Walker는 Teensy 4.1 (Logic MCU)과 Arduino Nano 33 BLE (Coms MCU) 간 SPI 통신을 사용.
STM32로 전환 시에도 SPI 통신 유지.

#### SPI 핀 옵션

| 페리페럴 | SCK | MOSI | MISO | NSS | AF | 버스 |
|----------|-----|------|------|-----|-----|------|
| **SPI1** | PA5, PB3 | PA7, PB5 | PA6, PB4 | PA4, PA15 | AF5 | APB2 |
| **SPI2** | PB10, PB13 | PB15, PC3 | PB14, PC2 | PB12, PB4 | AF5 | APB1 |
| **SPI3** | PB3, PC10 | PB5, PC12 | PB4, PC11 | PA4, PA15 | AF6 | APB1 |
| **SPI4** | PE2, PE12 | PE6, PE14 | PE5, PE13 | PE4, PE11 | AF5 | APB2 |

**AR_Walker 권장 매핑:**
- SPI1 (Master):
  - SCK: **PA5** (AF5)
  - MOSI: **PA7** (AF5)
  - MISO: **PA6** (AF5)
  - CS: **PA4** (GPIO, 소프트웨어 제어)
  - IRQ: **PC13** (GPIO 인터럽트 입력)

#### CubeMX 설정

1. Connectivity → SPI1 활성화
2. Mode: **Full-Duplex Master**
3. Parameter Settings:
   - Data Size: **8 bit**
   - First Bit: **MSB First**
   - Prescaler: **16** (APB2 120MHz / 16 = 7.5 MHz)
   - Clock Polarity (CPOL): **Low** (모드 0 또는 8에 맞춤)
   - Clock Phase (CPHA): **1 Edge** (모드 0) 또는 **2 Edge** (모드 8)
   - NSS: **Software** (CS를 GPIO로 직접 제어)

> **CPOL/CPHA 모드**: 현재 Teensy의 SPI 모드 8에 맞춰야 한다.
> SPI Mode 0 = CPOL:0 CPHA:0, Mode 1 = CPOL:0 CPHA:1,
> Mode 2 = CPOL:1 CPHA:0, Mode 3 = CPOL:1 CPHA:1

#### HAL 코드 예제

```c
SPI_HandleTypeDef hspi1;

/* SPI 송수신 (블로킹) */
void SPI_TransmitReceive(uint8_t *tx_data, uint8_t *rx_data, uint16_t size)
{
    // CS LOW (통신 시작)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, size, 100);

    // CS HIGH (통신 종료)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

/* SPI DMA 송수신 (비블로킹, 권장) */
void SPI_TransmitReceive_DMA(uint8_t *tx_data, uint8_t *rx_data, uint16_t size)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive_DMA(&hspi1, tx_data, rx_data, size);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        // 수신 데이터 처리
        process_coms_mcu_data();
    }
}
```

---

### 5.4 ADC (토크 센서, 각도 센서)

AR_Walker의 로드셀(토크 센서)과 각도 센서는 아날로그 전압을 출력한다.
STM32H7의 ADC는 최대 16비트 분해능을 지원 (현재 Teensy는 12비트 사용).

#### ADC 채널과 핀 매핑

| ADC | 채널 | 핀 | 용도 (AR_Walker) |
|-----|------|-----|-----------------|
| ADC1 | IN0 | PA0 | 토크센서 Left (현재 A16) |
| ADC1 | IN1 | PA1 | Maxon 전류 Left |
| ADC1 | IN2 | PA2 | Maxon 전류 Right |
| ADC1 | IN6 | PA6 | 토크센서 Right (현재 A6) |
| ADC1 | IN12 | PC2 | 각도센서 Right (현재 A12) |
| ADC1 | IN13 | PC3 | 각도센서 Left (현재 A13) |
| ADC1 | IN14 | PC4 | (예비) |
| ADC1 | IN15 | PC5 | (예비) |

> ⚠️ **주의**: PA6를 ADC로 사용하면 SPI1_MISO로는 사용 불가.
> 이 경우 SPI1_MISO를 PB4로 재배치하거나, 토크센서를 다른 ADC 채널(PC4 등)로 이동해야 한다.
> → 7장 핀 매핑 전략에서 이 충돌을 해결한다.

#### CubeMX 설정

1. Analog → ADC1 활성화
2. IN0, IN6, IN12, IN13 등 필요한 채널 체크
3. Parameter Settings:
   - Clock Prescaler: **Asynchronous clock mode divided by 4**
   - Resolution: **12 bit** (Teensy와 동일) 또는 **16 bit** (더 정밀)
   - Scan Conversion Mode: **Enable** (여러 채널 순차 변환)
   - Continuous Conversion Mode: **Enable** (계속 변환)
   - DMA Continuous Requests: **Enable**
   - Number of Conversions: **4** (사용할 채널 수)
4. DMA Settings → ADC1 → DMA Stream 추가 → Circular 모드

#### HAL 코드 예제

```c
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

// DMA로 수신할 버퍼 (채널 4개 × 값)
// D2 SRAM에 배치해야 DMA가 접근 가능!
__attribute__((section(".RAM_D2")))
volatile uint16_t adc_values[4];
// [0]=토크L, [1]=토크R, [2]=각도R, [3]=각도L

/* ADC + DMA 시작 */
void ADC_Start(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_values, 4);
    // 이후 adc_values[]는 자동으로 업데이트됨
}

/* ADC 값 읽기 (아무 때나 호출 가능) */
float get_torque_left_voltage(void)
{
    // 12비트 ADC: 0~4095 → 0~3.3V
    return (float)adc_values[0] * 3.3f / 4096.0f;
}

float get_torque_left_Nm(void)
{
    float voltage = get_torque_left_voltage();
    // 로드셀 캘리브레이션 적용
    // 현재 Config.h: AI_CNT_TO_V = 3.3 / 4096
    return (voltage - bias) * sensitivity;
}

/* ADC 변환 완료 콜백 (DMA 사용 시 자동 호출) */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        // 새로운 ADC 데이터 준비됨
        // DMA Circular 모드이므로 자동으로 다음 변환 시작
    }
}
```

> **중요: DMA 버퍼 위치**
> STM32H7에서 DMA1/2는 **D2 도메인의 SRAM**에만 접근 가능하다.
> AXI SRAM이나 DTCM에 DMA 버퍼를 배치하면 동작하지 않는다!
> 링커 스크립트에 `.RAM_D2` 섹션을 추가하고, `__attribute__((section(".RAM_D2")))`로 배치한다.

---

### 5.5 PWM / Timer (모터 제어 신호)

Maxon 모터 드라이버는 PWM 신호로 속도/토크 명령을 받는다.
현재 Teensy에서 `analogWrite()`로 PWM을 출력하는 것을 STM32 타이머로 구현한다.

#### 타이머 종류

| 분류 | 타이머 | 특징 | PWM 채널 |
|------|--------|------|---------|
| **Advanced** | TIM1, TIM8 | 데드타임, 브레이크 기능 | 각 4채널 |
| **General Purpose (32bit)** | TIM2, TIM5 | 32비트 카운터 | 각 4채널 |
| **General Purpose (16bit)** | TIM3, TIM4 | 범용 | 각 4채널 |
| **General Purpose (1ch)** | TIM15, TIM16, TIM17 | 단일 채널 | 각 1~2채널 |
| **Basic** | TIM6, TIM7 | PWM 불가, 인터럽트/DAC 트리거용 | 없음 |

**AR_Walker 권장:**
- Maxon PWM Left: **TIM1_CH1** → **PE9** (AF1)
- Maxon PWM Right: **TIM1_CH2** → **PE11** (AF1)
- TIM1은 Advanced 타이머로 정밀한 PWM 출력에 적합

#### CubeMX 설정

1. Timers → TIM1 → Channel 1: **PWM Generation CH1**
2. Timers → TIM1 → Channel 2: **PWM Generation CH2**
3. Parameter Settings:
   - Prescaler: **239** (타이머 클럭 240MHz / (239+1) = 1 MHz)
   - Counter Period (ARR): **999** (1MHz / (999+1) = **1 kHz** PWM 주파수)
   - Pulse (CCR): **500** (50% 듀티 = 중립값)
   - PWM Mode: **PWM Mode 1**
   - CH Polarity: **High** (Active High)

> **PWM 주파수 계산:**
> `PWM_freq = Timer_CLK / ((PSC+1) * (ARR+1))`
> `= 240MHz / (240 * 1000) = 1 kHz`
>
> **듀티 사이클 계산:**
> `Duty = CCR / (ARR+1) * 100%`
> `= 500 / 1000 * 100% = 50%` (중립)

#### HAL 코드 예제

```c
TIM_HandleTypeDef htim1;

/* PWM 시작 */
void Motor_PWM_Start(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // Left motor
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);  // Right motor
}

/* 듀티 사이클 변경 (토크 명령) */
void Motor_SetDuty(uint32_t channel, float duty_percent)
{
    // duty_percent: 0.0 ~ 100.0
    uint32_t pulse = (uint32_t)(duty_percent / 100.0f * (htim1.Init.Period + 1));
    __HAL_TIM_SET_COMPARE(&htim1, channel, pulse);
}

/* Maxon 모터 토크 명령 → PWM 변환 */
void Maxon_SetTorque(float torque_left, float torque_right)
{
    // 현재 Board.h의 중립값 기준으로 듀티 계산
    // maxon_pwm_neutral_val을 50%로 가정
    float duty_left  = 50.0f + torque_left * scale_factor;
    float duty_right = 50.0f + torque_right * scale_factor;

    // 상한/하한 제한 (안전)
    duty_left  = fminf(fmaxf(duty_left,  10.0f), 90.0f);
    duty_right = fminf(fmaxf(duty_right, 10.0f), 90.0f);

    Motor_SetDuty(TIM_CHANNEL_1, duty_left);
    Motor_SetDuty(TIM_CHANNEL_2, duty_right);
}

/* 모터 정지 (안전 함수) */
void Motor_Stop(void)
{
    // 중립값으로 복귀
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 500);  // 50%
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 500);

    // PWM 출력 정지
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
}
```

---

## 6. STM32CubeMX 실전 설정 과정

![CubeMX 핀 설정 화면](/images/study/stm32/cubemx-pinout.png)
*CubeMX 핀 설정 화면 — 스크린샷 또는 Gemini로 생성 필요*


CubeMX는 STM32의 핀 배치, 클럭, 페리페럴을 GUI로 설정하고 초기화 코드를 자동 생성하는 도구이다.
STM32CubeIDE에 내장되어 있다.

### Step 1: 프로젝트 생성 & 칩 선택

1. STM32CubeIDE → **File** → **New** → **STM32 Project**
2. **MCU/MPU Selector** 탭에서 검색: `STM32H743VITx`
3. 칩 선택 후 **Next**
4. Project Name: `AR_Walker_STM32` (또는 `H-Walker_STM32_Test`)
5. Targeted Language: **C**
6. Targeted Binary Type: **Executable**
7. Targeted Project Type: **STM32Cube**
8. **Finish** → `.ioc` 파일이 열리며 핀 설정 화면 표시

### Step 2: 핀 할당 (Pinout & Configuration)

`.ioc` 에디터의 칩 그래픽에서 핀을 클릭하여 기능을 할당한다.

**설정 순서 (권장):**

1. **디버그 핀 확보**: System Core → SYS → Debug: **Serial Wire** (PA13/PA14 자동 할당)
2. **클럭 소스**: System Core → RCC → HSE: **Crystal/Ceramic Resonator**
3. **FDCAN1**: Connectivity → FDCAN1 → Activated
   - TX: PD1, RX: PD0 (자동 또는 수동 선택)
4. **UART (IMU)**: Connectivity → UART4 → Mode: Asynchronous
   - RX: PA1 (TX 불필요하면 비활성화)
5. **UART (디버그)**: Connectivity → USART3 → Mode: Asynchronous
   - TX: PD8, RX: PD9
6. **SPI1**: Connectivity → SPI1 → Mode: Full-Duplex Master
   - SCK: PA5, MOSI: PB5, MISO: PB4 (PA6/PA7 ADC용으로 남겨두기)
7. **ADC1**: Analog → ADC1 → IN0, IN1, IN2, IN6 (또는 IN14/IN15) 활성화
   - PA0, PA1 주의: UART4 RX와 충돌 시 ADC 채널 재배치
8. **TIM1 PWM**: Timers → TIM1 → CH1: PWM Generation, CH2: PWM Generation
   - CH1: PE9, CH2: PE11
9. **GPIO 출력**: 각 핀을 클릭 → GPIO_Output 선택
   - LED, Motor Enable, Motor Stop 핀들
10. **GPIO 입력**: 모터 에러 핀 등

**핀 충돌 확인:**
- CubeMX에서 핀이 **노란색** = 경고 (대체 가능)
- **빨간색** = 충돌 (해결 필수)
- 좌측 패널에서 **"Pinout Conflict"** 메시지 확인

### Step 3: 클럭 설정 (Clock Configuration)

1. 상단 탭에서 **Clock Configuration** 클릭
2. 좌측 Input frequency: **8** (MHz, 사용할 크리스탈에 맞춤)
3. PLL Source Mux: **HSE** 선택
4. DIVM1: 1, DIVN1: 120, DIVP1: 2 입력
5. System Clock Mux: **PLLCLK** 선택
6. HCLK: **240 MHz** 확인 (자동 계산)
7. 각 APB 클럭이 120 MHz인지 확인
8. 빨간색 경고가 있으면 **"Resolve Clock Issues"** 버튼 클릭

### Step 4: 페리페럴 파라미터 설정

좌측 **Configuration** 패널에서 각 페리페럴의 상세 설정:

#### FDCAN1 파라미터
```
Mode                    : Normal
Frame Format            : Classic (CAN 2.0)
Auto Retransmission     : Enable
Nominal Prescaler       : 10
Nominal Sync Jump Width : 1
Nominal Time Seg1       : 5
Nominal Time Seg2       : 6
→ Bit Rate = 120MHz / (10 × (1+5+6)) = 1 Mbps
```

#### ADC1 파라미터
```
Clock Prescaler         : Asynchronous clock mode divided by 4
Resolution              : ADC 12-bit resolution (또는 16-bit)
Scan Conversion Mode    : Enable
Continuous Conv Mode    : Enable
DMA Continuous Requests : Enable
Number of Conversion    : (사용할 채널 수)
```

#### DMA 설정
각 페리페럴의 DMA Settings 탭에서:
- ADC1 → DMA Stream 추가 → Mode: **Circular**
- UART4_RX → DMA Stream 추가 → Mode: **Circular**

#### NVIC (인터럽트 우선순위)
```
인터럽트           우선순위(0=최고)  용도
FDCAN1_IT0         1               CAN 수신 (모터 응답 — 최우선)
TIM6_DAC           2               제어 루프 타이머 (500Hz)
DMA_ADCx           3               ADC 변환 완료
UART4_IRQn         4               IMU 데이터 수신
SPI1_IRQn          5               통신 MCU 데이터
EXTI_IRQn          6               GPIO 인터럽트 (에러 등)
```

### Step 5: 프로젝트 설정

1. **Project Manager** 탭 클릭
2. Project Settings:
   - Toolchain: **STM32CubeIDE**
   - Generate Under Root: 체크
3. Code Generator:
   - **"Generate peripheral initialization as a pair of '.c/.h' files per peripheral"** → 체크 (권장)
   - **"Keep User Code when re-generating"** → 체크 (필수!)
   - **"Delete previously generated files when not re-generated"** → 체크

### Step 6: 코드 생성

1. **Project** → **Generate Code** (또는 Alt+K, Cmd+K)
2. 생성되는 파일 구조:

```
AR_Walker_STM32/
├── Core/
│   ├── Inc/
│   │   ├── main.h              ← GPIO 핀 define (CubeMX 자동)
│   │   ├── stm32h7xx_hal_conf.h
│   │   └── stm32h7xx_it.h
│   └── Src/
│       ├── main.c              ← ★ 메인 코드 여기에 작성
│       ├── stm32h7xx_hal_msp.c ← 페리페럴 MSP 초기화
│       └── stm32h7xx_it.c      ← 인터럽트 핸들러
├── Drivers/
│   ├── CMSIS/                  ← ARM 코어 헤더
│   └── STM32H7xx_HAL_Driver/   ← HAL 라이브러리
└── STM32H743VITX_FLASH.ld      ← 링커 스크립트
```

### USER CODE 블록 규칙

CubeMX가 코드를 재생성해도 보존되는 영역:

```c
/* USER CODE BEGIN Includes */
#include "motor_control.h"    // ✅ 안전!
/* USER CODE END Includes */

// ❌ 여기에 쓰면 재생성 시 삭제됨!

/* USER CODE BEGIN 0 */
void my_init(void) { }        // ✅ 안전!
/* USER CODE END 0 */
```

> **최선의 방법**: `Core/Src/`에 별도 `.c` 파일을 만들어 유저 코드를 작성한다.
> 예: `motor_control.c`, `sensor_read.c`, `can_protocol.c`
> → CubeMX가 건드리지 않으므로 100% 안전.
> (자세한 내용은 [README.md](README.md)의 "자동 생성 코드와 유저 코드 관리" 섹션 참고)

---

## 7. AR_Walker 핀 매핑 전략

![Teensy → STM32 핀 매핑](/images/study/stm32/pin-mapping.png)
*Teensy 4.1에서 STM32H743으로의 핀 매핑 비교도 — Gemini로 생성 필요*


### 7.1 핀 배치 원칙

1. **AF 충돌 방지**: 하나의 핀에는 하나의 AF만 사용 가능
2. **전원/그라운드 확보**: LQFP-100의 18개 전원 핀 모두 적절히 연결
3. **노이즈 분리**: ADC 입력 핀은 고속 디지털 신호(CAN, SPI)와 물리적으로 분리
4. **디버거 보호**: PA13 (SWDIO), PA14 (SWCLK)는 절대 다른 용도로 사용하지 않음
5. **미사용 핀 처리**: Analog 모드 (소비전류 최소화) 또는 Output Low로 설정
6. **BOOT0 핀**: GND에 연결 (일반 부트 = Flash에서 실행)

### 7.2 Teensy → STM32 핀 매핑 테이블

현재 `Board.h`의 Teensy 4.1 핀 설정을 STM32H743VITx로 매핑한 최종 테이블:

#### 통신 페리페럴

| 기능 | Teensy 핀 | STM32 핀 | AF | 페리페럴 | 비고 |
|------|-----------|----------|-----|---------|------|
| CAN TX | 22 | **PD1** | AF9 | FDCAN1_TX | 모터 CAN 버스 |
| CAN RX | 23 | **PD0** | AF9 | FDCAN1_RX | 모터 CAN 버스 |
| IMU UART RX | 16 (RX4) | **PD9** | AF7 | USART3_RX | IMU 데이터 수신 |
| IMU UART TX | N/C | **PD8** | AF7 | USART3_TX | (디버그 겸용 가능) |
| SPI SCK | (implicit) | **PA5** | AF5 | SPI1_SCK | Coms MCU 통신 |
| SPI MOSI | 11 | **PB5** | AF5 | SPI1_MOSI | PA7은 ADC용으로 보존 |
| SPI MISO | (implicit) | **PB4** | AF5 | SPI1_MISO | PA6은 ADC용으로 보존 |
| SPI CS | 10 | **PA15** | GPIO | — | 소프트웨어 CS |
| SPI IRQ | 34 | **PE0** | GPIO | EXTI0 | 인터럽트 입력 |
| SPI RST | 4 | **PE1** | GPIO | — | Coms MCU 리셋 |
| Serial TX | 1 | **PA9** | AF7 | USART1_TX | (디버그/PC 통신) |
| Serial RX | 0 | **PA10** | AF7 | USART1_RX | (디버그/PC 통신) |

#### 아날로그 입력 (ADC)

| 기능 | Teensy 핀 | STM32 핀 | 채널 | 페리페럴 | 비고 |
|------|-----------|----------|------|---------|------|
| 토크센서 Left | A16 | **PA0** | IN0 | ADC1 | 로드셀 L |
| 토크센서 Right | A6 | **PA3** | IN3 | ADC1 | 로드셀 R (PA6 대신 PA3) |
| 각도센서 Left | A13 | **PC3** | IN13 | ADC1 | 좌측 발목 각도 |
| 각도센서 Right | A12 | **PC2** | IN12 | ADC1 | 우측 발목 각도 |
| Maxon 전류 Left | (maxon_current) | **PA1** | IN1 | ADC1 | 좌측 모터 전류 |
| Maxon 전류 Right | (maxon_current) | **PA2** | IN2 | ADC1 | 우측 모터 전류 |
| (예비 1) | — | **PC4** | IN14 | ADC1 | 확장용 |
| (예비 2) | — | **PC5** | IN15 | ADC1 | 확장용 |

#### PWM 출력

| 기능 | Teensy 핀 | STM32 핀 | AF | 페리페럴 | 비고 |
|------|-----------|----------|-----|---------|------|
| Maxon PWM Left | (maxon_ctrl_L) | **PE9** | AF1 | TIM1_CH1 | 좌측 모터 |
| Maxon PWM Right | (maxon_ctrl_R) | **PE11** | AF1 | TIM1_CH2 | 우측 모터 |

#### GPIO 출력

| 기능 | Teensy 핀 | STM32 핀 | 설정 | 비고 |
|------|-----------|----------|------|------|
| Status LED Red | 14 | **PB0** | Output PP, Low Speed | RGB LED |
| Status LED Green | 25 | **PB1** | Output PP, Low Speed | RGB LED |
| Status LED Blue | 24 | **PB2** | Output PP, Low Speed | RGB LED |
| Sync LED | 15 | **PB10** | Output PP, Low Speed | 동기화 LED |
| Motor Stop | 9 | **PC6** | Output PP, Pull-Down | 긴급 정지 |
| Motor Enable L0 | 28 | **PD3** | Output PP, Pull-Down | 좌측 관절 0 |
| Motor Enable L1 | 29 | **PD4** | Output PP, Pull-Down | 좌측 관절 1 |
| Motor Enable R0 | 8 | **PD5** | Output PP, Pull-Down | 우측 관절 0 |
| Motor Enable R1 | 7 | **PD6** | Output PP, Pull-Down | 우측 관절 1 |
| Sync Default | 5 | **PC7** | Output PP | 동기화 기본 |
| Speed Check | 33 | **PC8** | Output PP | 속도 측정용 토글 핀 |

#### GPIO 입력

| 기능 | Teensy 핀 | STM32 핀 | 설정 | 비고 |
|------|-----------|----------|------|------|
| Maxon Error Left | (maxon_err_L) | **PE2** | Input, Pull-Up | 에러 감지 (액티브 로우) |
| Maxon Error Right | (maxon_err_R) | **PE3** | Input, Pull-Up | 에러 감지 (액티브 로우) |

#### 시스템 핀 (변경 불가)

| 기능 | STM32 핀 | 비고 |
|------|----------|------|
| SWDIO (디버거) | PA13 | 절대 변경 금지 |
| SWCLK (디버거) | PA14 | 절대 변경 금지 |
| HSE 크리스탈 IN | PH0 (pin 12) | 8MHz 크리스탈 |
| HSE 크리스탈 OUT | PH1 (pin 13) | 8MHz 크리스탈 |
| NRST | pin 14 (NRST) | 리셋 버튼 |
| BOOT0 | pin 94 | GND 연결 (Flash 부트) |

### 7.3 AF 충돌 검증

위 매핑에서 주요 충돌 해결 사항:

| 문제 | 원인 | 해결 |
|------|------|------|
| PA6을 ADC와 SPI1_MISO에 동시 사용 불가 | AF 충돌 | SPI1_MISO를 **PB4** (AF5)로 이동 |
| PA7을 ADC와 SPI1_MOSI에 동시 사용 불가 | AF 충돌 | SPI1_MOSI를 **PB5** (AF5)로 이동 |
| PA1을 ADC(전류센싱)와 UART4_RX에 동시 불가 | AF 충돌 | IMU UART를 **USART3** (PD8/PD9)로 변경 |
| PA6을 토크센서 R ADC로 사용 시 SPI 불가 | AF 충돌 | 토크센서 R을 **PA3** (ADC1_IN3)로 이동 |

### 7.4 핀 사용 현황 요약

| 포트 | 사용된 핀 | 미사용 핀 | 비고 |
|------|----------|----------|------|
| GPIOA | PA0-5, PA9-10, PA13-15 | PA6-8, PA11-12 | PA11/12는 USB용으로 예비 |
| GPIOB | PB0-2, PB4-5, PB10 | PB3, PB6-9, PB11-15 | 여유 있음 |
| GPIOC | PC2-8 | PC0-1, PC9-13 | PC13은 WKUP 가능 |
| GPIOD | PD0-1, PD3-6, PD8-9 | PD2, PD7, PD10-15 | 여유 있음 |
| GPIOE | PE0-3, PE9, PE11 | PE4-8, PE10, PE12-15 | 여유 있음 |

> **총 사용 핀**: 약 40개 / **여유 핀**: 약 36개 — 향후 확장 충분

### 7.5 핀맵 문서화

최종 확정된 핀 매핑은 `templates/hardware_pinmap_template.md` 양식에 맞춰
`Documentation/Hardware/AR_Walker_STM32_Pinmap.md`로 작성한다.

---

## 8. 보드 브링업

![보드 브링업 프로세스](/images/study/stm32/bringup-flow.png)
*보드 브링업 9단계 플로차트 — Gemini로 생성 필요*
 (Board Bring-up) 프로세스

보드를 제작한 후, 페리페럴을 하나씩 테스트하며 정상 동작을 확인하는 과정이다.
**절대 모든 것을 한 번에 테스트하지 마라** — 문제 발생 시 원인을 찾을 수 없다.

### 8.1 단계별 테스트 순서

```
┌─────────────────────────────────────────────────┐
│  Step 1: 전원 확인                                │
│  → VDD, VDDA, VCAP 전압 측정                     │
│  → 쇼트 없는지 확인                               │
├─────────────────────────────────────────────────┤
│  Step 2: 클럭 확인                                │
│  → HSE 크리스탈 발진 확인                          │
│  → MCO 핀으로 클럭 출력하여 오실로스코프 측정       │
├─────────────────────────────────────────────────┤
│  Step 3: LED 테스트 (GPIO Output)                │
│  → LED 깜빡이기 = 전원+클럭+Flash+GPIO 정상       │
├─────────────────────────────────────────────────┤
│  Step 4: UART 테스트 (PC 통신)                    │
│  → printf 출력 확인                               │
│  → 양방향 에코 테스트                              │
├─────────────────────────────────────────────────┤
│  Step 5: CAN 테스트                               │
│  → 루프백 모드 (자기 자신에게 송수신)               │
│  → 외부 CAN 장치 연결 테스트                       │
├─────────────────────────────────────────────────┤
│  Step 6: SPI 테스트                               │
│  → 로직 분석기로 SCK/MOSI 파형 확인                │
│  → Coms MCU와 데이터 교환                         │
├─────────────────────────────────────────────────┤
│  Step 7: ADC 테스트                               │
│  → 알려진 전압 (예: 1.65V) 입력 후 읽기            │
│  → 모든 채널 순차 확인                             │
├─────────────────────────────────────────────────┤
│  Step 8: PWM 테스트                               │
│  → 오실로스코프로 주파수/듀티 확인                  │
│  → 모터 드라이버 연결 전 파형만 먼저 확인           │
├─────────────────────────────────────────────────┤
│  Step 9: 통합 테스트                               │
│  → 모든 페리페럴 동시 동작                         │
│  → 500Hz 제어 루프 타이밍 측정                     │
└─────────────────────────────────────────────────┘
```

### 8.2 Step 1: 전원 확인

**전원 인가 전:**
- 멀티미터로 VDD-VSS, VDDA-VSSA 간 **저항 측정** → 쇼트 확인
- PCB 납땜 상태 육안 검사 (브릿지, 미납 등)

**전원 인가 후:**
| 핀 | 기대 전압 | 허용 범위 | 비고 |
|-----|----------|----------|------|
| VDD (여러 핀) | 3.3V | 3.0~3.6V | 디지털 전원 |
| VDDA | 3.3V | 3.0~3.6V | 아날로그 전원 (노이즈 주의) |
| VREF+ | 3.3V | = VDDA | ADC 기준 전압 |
| VCAP1 | ~1.2V | 자동 | 내부 레귤레이터 출력, 1uF 캡 연결 |

> **VCAP1이 ~1.2V가 아니면** 내부 레귤레이터가 동작하지 않는 것 → 코어가 동작 불가.
> 원인: VCAP 핀에 캐패시터 미연결, 또는 VDD 전원 불량.

### 8.3 Step 2: 클럭 확인

LED 테스트 전에 클럭이 정상인지 먼저 확인:

```c
/* CubeMX에서 MCO1 출력 활성화: RCC → MCO1 → HSE */
/* PA8 (MCO1 핀)에서 8MHz 클럭 출력 → 오실로스코프로 측정 */

/* 또는 코드에서 직접 설정: */
HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
// PA8에서 8MHz 사각파가 나오면 HSE 크리스탈 정상!

HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLL1QCLK, RCC_MCODIV_4);
// PLL1Q / 4 출력 → 240MHz/4 = 60MHz 확인 가능
```

### 8.4 Step 3: LED 테스트

4.7절의 LED 깜빡이기 코드 사용.

**확인 사항:**
- [ ] LED가 정확히 500ms 간격으로 깜빡이는가?
- [ ] 다른 GPIO 핀의 LED도 동작하는가? (RGB LED 각 색상)
- [ ] SWD 디버거로 코드 다운로드 및 디버깅이 되는가?

### 8.5 Step 4: UART 테스트

```c
/* USER CODE BEGIN 2 */
printf("AR_Walker STM32 Board Test\r\n");
printf("SYSCLK: %lu MHz\r\n", HAL_RCC_GetSysClockFreq() / 1000000);
printf("HCLK:   %lu MHz\r\n", HAL_RCC_GetHCLKFreq() / 1000000);
/* USER CODE END 2 */

while (1)
{
    /* USER CODE BEGIN 3 */
    // 에코 테스트: 수신한 데이터를 그대로 송신
    uint8_t rx_byte;
    if (HAL_UART_Receive(&huart3, &rx_byte, 1, 10) == HAL_OK)
    {
        HAL_UART_Transmit(&huart3, &rx_byte, 1, 10);
    }
    /* USER CODE END 3 */
}
```

**PC에서 확인**: 시리얼 터미널 (PuTTY, minicom, Arduino Serial Monitor)로 115200 baud 연결.

### 8.6 Step 5: CAN 루프백 테스트

```c
/* CubeMX에서 FDCAN1 Mode를 "Internal LoopBack"으로 설정 */
/* → 외부 CAN 트랜시버 없이 자기 자신에게 메시지를 보내 수신 확인 */

void CAN_LoopbackTest(void)
{
    FDCAN_TxHeaderTypeDef tx_header;
    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t tx_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t rx_data[8] = {0};

    tx_header.Identifier = 0x123;
    tx_header.IdType = FDCAN_STANDARD_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = FDCAN_DLC_BYTES_8;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;

    // 메시지 송신
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, tx_data);
    HAL_Delay(10);

    // 메시지 수신 확인
    if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
    {
        if (rx_header.Identifier == 0x123 && rx_data[0] == 0x01)
        {
            printf("CAN Loopback Test: PASSED!\r\n");
        }
    }
    else
    {
        printf("CAN Loopback Test: FAILED!\r\n");
    }
}
```

### 8.7 Step 6~8: SPI, ADC, PWM 테스트

각각 독립적으로 테스트한다. 순서는 중요하지 않지만, 하나씩 확인한다.

**SPI 테스트:**
```c
// 루프백: MOSI와 MISO를 점퍼 와이어로 연결
uint8_t tx = 0xA5, rx = 0x00;
HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100);
printf("SPI Loopback: TX=0x%02X, RX=0x%02X %s\r\n",
       tx, rx, (tx == rx) ? "PASS" : "FAIL");
```

**ADC 테스트:**
```c
// PA0에 1.65V (VDD/2) 연결 후:
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, 100);
uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
float voltage = adc_val * 3.3f / 4096.0f;
printf("ADC IN0: %lu (%.3f V, expected ~1.65V)\r\n", adc_val, voltage);
```

**PWM 테스트:**
```c
// PE9에서 1kHz, 50% 듀티 PWM 출력
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
// → 오실로스코프로 PE9 핀 측정: 1kHz, 50% 확인
// 듀티 변경 테스트:
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 250);  // 25%
HAL_Delay(2000);
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 750);  // 75%
```

### 8.8 Step 9: 통합 테스트

모든 개별 테스트 통과 후, 전체를 동시에 구동:

```c
/* 500Hz 제어 루프 (TIM6 인터럽트 사용) */
// CubeMX: TIM6, Prescaler=239, Period=999 → 240MHz/(240*1000) = 1kHz
// 인터럽트에서 500Hz 카운터 사용

volatile uint8_t control_loop_flag = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6)
    {
        control_loop_flag = 1;
    }
}

/* main.c while(1) 내부 */
while (1)
{
    if (control_loop_flag)
    {
        control_loop_flag = 0;

        // 타이밍 측정 시작
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);  // Speed Check 핀

        // 1. ADC 읽기 (DMA이므로 즉시 사용 가능)
        float torque_L = get_torque_left_Nm();
        float torque_R = get_torque_right_Nm();

        // 2. IMU 데이터 확인 (DMA 수신 완료된 최신 값)
        float gyro_z = get_imu_gyro_z();

        // 3. 제어 알고리즘 실행
        float cmd_L = controller_update(torque_L, gyro_z);
        float cmd_R = controller_update(torque_R, gyro_z);

        // 4. 모터 명령 송신 (CAN 또는 PWM)
        CAN_SendMotorCommand(0x01, cmd_L);
        CAN_SendMotorCommand(0x02, cmd_R);

        // 5. SPI로 Coms MCU에 상태 전송
        SPI_SendStatus(torque_L, torque_R, cmd_L, cmd_R);

        // 타이밍 측정 끝
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    }
}
```

**통합 테스트 확인 사항:**
- [ ] Speed Check 핀(PC8)의 HIGH 구간이 2ms(= 500Hz 주기) 미만인가? (제어 루프가 시간 내 완료)
- [ ] 모든 ADC 채널이 정상 값을 읽는가?
- [ ] CAN 송수신이 제어 루프와 동기화되어 동작하는가?
- [ ] UART 디버그 출력이 제어 루프에 영향을 주지 않는가?
- [ ] SPI 통신이 정상인가?
- [ ] 장시간 (수 분~수 시간) 운행해도 안정적인가?

---

## 부록: 자주 사용하는 HAL 함수 레퍼런스

| 카테고리 | 함수 | 설명 |
|---------|------|------|
| **시스템** | `HAL_Init()` | HAL 초기화, SysTick 설정 |
| | `HAL_Delay(ms)` | 밀리초 대기 (블로킹) |
| | `HAL_GetTick()` | 시스템 시작 후 밀리초 카운터 |
| **GPIO** | `HAL_GPIO_Init(port, &init)` | GPIO 초기화 |
| | `HAL_GPIO_WritePin(port, pin, state)` | 핀 출력 |
| | `HAL_GPIO_ReadPin(port, pin)` | 핀 읽기 |
| | `HAL_GPIO_TogglePin(port, pin)` | 핀 토글 |
| **UART** | `HAL_UART_Transmit(&h, data, len, timeout)` | 송신 (블로킹) |
| | `HAL_UART_Receive(&h, data, len, timeout)` | 수신 (블로킹) |
| | `HAL_UART_Transmit_DMA(&h, data, len)` | DMA 송신 |
| | `HAL_UARTEx_ReceiveToIdle_DMA(&h, data, len)` | DMA 수신 (Idle 감지) |
| **SPI** | `HAL_SPI_TransmitReceive(&h, tx, rx, len, timeout)` | 송수신 |
| | `HAL_SPI_TransmitReceive_DMA(&h, tx, rx, len)` | DMA 송수신 |
| **ADC** | `HAL_ADC_Start(&h)` | ADC 변환 시작 |
| | `HAL_ADC_Start_DMA(&h, data, len)` | DMA ADC 시작 |
| | `HAL_ADC_GetValue(&h)` | 변환 결과 읽기 |
| **CAN** | `HAL_FDCAN_Start(&h)` | FDCAN 시작 |
| | `HAL_FDCAN_AddMessageToTxFifoQ(&h, &hdr, data)` | 메시지 송신 |
| | `HAL_FDCAN_GetRxMessage(&h, fifo, &hdr, data)` | 메시지 수신 |
| | `HAL_FDCAN_ConfigFilter(&h, &filter)` | 수신 필터 설정 |
| **Timer** | `HAL_TIM_PWM_Start(&h, ch)` | PWM 출력 시작 |
| | `__HAL_TIM_SET_COMPARE(&h, ch, val)` | PWM 듀티 변경 |
| | `HAL_TIM_Base_Start_IT(&h)` | 타이머 인터럽트 시작 |
| **클럭** | `HAL_RCC_GetSysClockFreq()` | SYSCLK 주파수 확인 |
| | `HAL_RCC_GetHCLKFreq()` | AHB 클럭 확인 |

---

## 참고 자료

| 자료 | 설명 |
|------|------|
| **STM32H743 Datasheet** (DS12110) | 핀아웃, AF 테이블, 전기적 특성 |
| **STM32H7 Reference Manual** (RM0433) | 레지스터, 페리페럴 상세 동작 |
| **STM32H7 HAL User Manual** (UM2217) | HAL 함수 API 문서 |
| **AN5293** | STM32H7 FDCAN 사용 가이드 |
| **AN4031** | DMA 컨트롤러 사용 가이드 |
| **AR_Walker STM32_Setup/README.md** | 멀티 PC 개발 환경, USER CODE 관리 |
| **AR_Walker STM32_Setup/project_structure.md** | 모노레포 프로젝트 구조 |
| **AR_Walker Board.h** | 현재 Teensy 핀 설정 (매핑 원본) |

---

> **최종 업데이트**: 2026-04-05
> **작성 기준 칩**: STM32H743VITx (LQFP-100)
> **프로젝트**: AR_Walker (보행 보조 로봇 외골격)
