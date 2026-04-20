---
aliases: [Comms Block]
tags: [type/block, phase/A, domain/comm]
type: schematic_block
phase: A
status: pending
kicad_sheet: comms.kicad_sch
---

# Comms

## 목적

외부 통신 인터페이스 모음:
- **CAN-FD** (모터 드라이버 CANopen)
- **Jetson UART + HW SYNC** (pose 다운링크)
- **EBIMU UART** (무선 IMU 수신기 입력)

## 핵심 부품

- [[TCAN1462]] CAN-FD 트랜시버 (SO-8)
- 120Ω 종단 + 점퍼
- PESD2CAN ESD
- JST-GH 5pin (CAN: H, L, GND, SHLD, +V)
- JST-GH 4pin (Jetson UART: TX, RX, GND, 3V3)
- JST-GH 3pin (SYNC: SYNC_OUT, SYNC_IN, GND)
- JST-GH 4pin (EBIMU: TX, RX, GND, 5V)

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| FDCAN1_RX | PD0 | |
| FDCAN1_TX | PD1 | |
| UART2_TX (Jetson) | PA2 | |
| UART2_RX (Jetson) | PA3 | |
| SYNC_OUT | PB14 | TIM PWM 출력 가능 |
| SYNC_IN | PB15 | EXTI |
| UART4_TX (EBIMU spare) | PC10 | |
| UART4_RX (EBIMU) | PC11 | |

## 관련 개념

- [[Hardware Sync for Jetson]]

## 관련 부품

- [[TCAN1462]]

## 데이터시트

- [ ] TI TCAN1462 데이터시트
- [ ] EBIMU 수신기 매뉴얼 (배선 규격 확인)

## 작업 체크리스트

- [ ] CAN 레퍼런스 회로 수집
- [ ] 풋프린트 JST-GH 시리즈 통일
- [ ] sheet 그리기
