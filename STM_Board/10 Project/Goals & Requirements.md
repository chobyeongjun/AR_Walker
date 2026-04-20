---
aliases: [Goals, Requirements]
tags: [project, goals]
---

# Goals & Requirements

## 목적

Cable-driven exosuit 의 정밀 토크 제어용 **커스텀 STM32 보드**를 직접 설계·제작.
기존 [[OpenExo (NAU)]] (Teensy 4.1 기반) 구조를 STM32로 이식·개선.

## 액추에이터

- **2× BLDC 모터** (정확 제어 위해 모델 선정은 Phase 5에서 확정 → [[Modular Phase A-B Strategy]])
- 드라이버: **Elmo 서보 드라이버 (CANopen)**
- 모터당 연속 전류: **15 A+**, 피크 30 A (사용자 확정)

## 호스트

- **NVIDIA Jetson Orin** — 고수준 제어·학습 추론
- 데이터 플로우: 카메라 → pose 추정 → MCU 로 내려보냄
- 통신: **UART (DMA) + [[Hardware Sync for Jetson|HW SYNC GPIO]]** (결정적·저지연)

## 제어 루프

- Current/torque loop: **≥ 1 kHz**
- Outer loop: ≥ 200 Hz

## 센서

- IMU: **EBIMU (E2BOX 무선)** — 수신기 UART를 MCU 에 직결 (온보드 IMU 없음)
- 로드셀 (케이블 텐션) × 2: [[ADS131M04]] (24-bit, 32kSPS, 4ch 동시 샘플)
- 케이블 위치 인코더 × 2: [[AS5048A]]
- 배터리: [[Battery SoC Estimation|INA228 + 쿨롱 카운팅]]

## 안전

- [[E-stop Dual Safety|E-stop 이중 안전]] (SW EXTI + HW AND gate)
- [[eFuse|eFuse (LTC4368-1)]] — OV/UV/RV/OCP/inrush 통합
- IWDG 워치독, 모터 enable 게이트, 과전류·과전압·과온 보호

## 폼팩터

- **wearable** → 가능한 컴팩트
- 목표: **50 × 50 mm, 6-layer** (→ [[PCB Stackup 6-layer]])
- 공격적 옵션: 45 × 45 mm, 8-layer

## 로깅

- 온보드 microSD (SDMMC 4-bit)
- 추가: ESP32 BLE 로 실시간 텔레메트리, Jetson UART 브로드캐스트

## 무선

- 온보드 [[ESP32-C3-MINI-1U]] (BLE 5.0 + WiFi)
- IPEX 외부 안테나 (wearable 신호 안정)

## Non-goals

- EtherCAT 미지원 (CAN-FD/CANopen 만)
- Ethernet 미지원 (PHY 제거로 사이즈 절약)
- 셀 밸런싱/충전 제어 (BMS는 배터리 팩 자체)

## 참고

- [[Block Diagram]]
- [[Phase Plan]]
- [[Size Budget]]
