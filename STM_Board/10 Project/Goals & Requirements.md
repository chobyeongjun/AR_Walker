---
aliases: [Goals, Requirements]
tags: [project, goals]
---

# Goals & Requirements

## 목적

Cable-driven exosuit 정밀 토크 제어용 **커스텀 STM32 보드**.
AR_Walker (Teensy 기반) → STM32 + Jetson 아키텍처로 전환.
이전 시도에서 **재생 에너지 / GND bounce 로 보드 반복 파손** — 이번엔 [[Regen Energy Protection]], [[GND Bounce Protection]] 등 보호 아키텍처 전면 반영.

## 액추에이터

- **2× BLDC 모터** (모터 모델 Phase 5 / [[Open Questions]])
- 드라이버: **Elmo 서보** (Gold Twitter / Solo Twitter / Whistle 중 전류 계산 후 선택)
- 인터페이스: **CANopen (CiA 402)** over [[ISO1050]] **격리** CAN
- 모터당 연속 전류: **15A+ 연속 / 30A 피크** (사용자 정보, 정확한 계산은 Phase B)
- 제어 모드: **Servo (전류) + SET_POS** (legacy 결정 — 풀리 r(θ) 가변 → MIT 토크 모드 사용 안 함)

## 호스트 / 비전

- **NVIDIA Jetson Orin NX 16GB** (157 TOPS)
- 카메라: **Stereolabs ZED X Mini** (GMSL2, IP67, 150g, 120fps)
- 연결: [[ZED X Mini]] ─GMSL2→ Jetson ─UART+SYNC→ MCU
- 역할: pose estimation (ZED SDK + CUDA), 고수준 제어 모드, 데이터 로깅
- MCU 통신: **UART (DMA, 1Mbps) + [[Hardware Sync for Jetson|HW SYNC GPIO]]**

## MCU

- **[[STM32H743VIT6]]** (LQFP100 14×14)
- 2MB Flash / 1MB RAM / DP-FPU / HRTIM / 2× FDCAN / USB FS
- Nucleo-H743ZI 로 Phase A 펌웨어 선 개발 가능

## 제어 루프

- **Current/torque loop: 500 Hz** (legacy 확정, AR_Walker 와 동일)
- Outer loop: ≥ 200 Hz
- 어드미턴스 (입각기) + 위치 PID (유각기)
- RTOS: FreeRTOS

## 센서

- **IMU**: [[EBIMU EBMotion V5]] (2.4GHz 무선, 1000Hz, Quaternion)
- **로드셀** (케이블 텐션) × 2: [[ADS131M04]] (24-bit, 32kSPS, 4ch 동시 샘플링) + [[REF5025]] 2.5V ref
- **케이블 위치 인코더** × 2: [[AS5048A]] (SPI 14-bit 자기식)
- **배터리** × 2: [[INA228]] (85V max, 쿨롱 카운터)

## 배터리 / 전원

- **6S Li-ion 25.2V 완충** (24V 공칭) ⭐ legacy 확정
- 48V 직렬 옵션 ([[Open Questions]]) — 모터 속도 요구사항에 따라

## 안전 / 보호 (이전 파손 원인 해결)

→ [[Regen Energy Protection]] · [[GND Bounce Protection]] · [[Brake Resistor Circuit]] · [[Inrush Current Limiting]] · [[Isolated CAN]] · [[E-stop Dual Safety]]

- 재생 에너지 1J / 4 모터 동시 릴리즈: 11,280µF 벌크 + [[P6KE33CA]] × 2 + 브레이크 저항 26.5V
- GND bounce 2.5V (50A/µs × 50nH): [[ISO1050]] + [[MGJ2D05050SC]] 갈바닉 격리
- 인러시 480A: [[SL08-2R005]] NTC × 2 직렬 + Omron G5V-1 bypass
- 역전압: [[DMP2305U]] + [[BZT52C12]] VGS 보호
- E-stop: SW EXTI + HW [[74LVC1G08]] AND gate 이중
- IWDG 워치독

## 폼팩터

- wearable → 컴팩트 필수
- 목표: **50 × 50 mm, 6-layer** ([[PCB Stackup 6-layer]])
- 공격적 옵션: 45 × 45 mm, 8-layer

## 로깅

- 온보드 microSD (SDMMC 4-bit)
- ESP32-C3-MINI-1U BLE 실시간 텔레메트리
- Jetson UART 브로드캐스트

## 무선 제어

- [[ESP32-C3-MINI-1U]] — BLE 5.0 + WiFi, IPEX 외부 안테나
- MCU UART 로 BLE 코프로세서 운용

## Non-goals

- EtherCAT 미지원 (CAN-FD/CANopen 만)
- Ethernet 미지원 (PHY 제거로 사이즈 절약)
- 셀 밸런싱/충전 제어 (BMS 는 배터리 팩 자체)
- MIT 토크 모드 (풀리 r(θ) 가변 이슈)

## 관련

- [[Block Diagram]]
- [[Phase Plan]]
- [[Size Budget]]
- [[RECONCILIATION]]
