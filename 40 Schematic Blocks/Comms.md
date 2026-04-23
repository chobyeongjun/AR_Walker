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

외부 통신 인터페이스:
- **격리 CAN** (Elmo 드라이버 CANopen) — [[ISO1050]] + [[MGJ2D05050SC]]
- **Jetson UART** (고수준 명령·텔레메트리)
- **Jetson HW SYNC GPIO** (시계 정합) — v3.10 사용자 재추가
- **EBIMU UART** (무선 IMU 수신기 입력)

→ [[Wireless]] (ESP32) 는 별도 블록

## ⚠️ 격리 CAN (legacy review 핵심)

[[TCAN1462]] (비격리) → **[[ISO1050]] (격리)** 로 변경.

이유: [[GND Bounce Protection]] · [[Isolated CAN]] · [[_legacy/RECONCILIATION]]
요약: 이전 보드가 GND bounce 2.5V 로 CAN TXD/RXD 반복 파손.

출처: `_legacy/EXOSUIT_PROTECTION.md`, `_legacy/BOARD_DESIGN_REVIEWED.md`

## 핵심 부품

### CAN 격리 체인 (FDCAN1)

- [[ISO1050]] 격리 CAN 트랜시버 (SOIC-16 wide, 5000 Vrms)
- [[MGJ2D05050SC]] 격리 DC/DC (5V→5V 2W, VCC2 공급)
- BZT52C5V6 5.6V 제너 (VCC2 절대최대 6.5V 보호)
- ACM2012-900 CMC (900Ω@100MHz, EMI 차단)
- SM712-02HTG TVS (500W, ±8kV ESD)
- 120Ω 종단 + 점퍼
- 33Ω × 2 MCU 측 댐핑
- 22Ω × 2 버스 측 댐핑
- PRTR5V0U2X ESD (MCU 측)
- JST-GH 5pin (CAN_H, CAN_L, GND, SHLD, +V tap)

### Jetson UART + HW SYNC

- JST-GH 4pin (TX, RX, GND, 3V3) — UART
- **JST-GH 3pin (SYNC_OUT, SYNC_IN, GND)** — 하드웨어 시계 정합 ([[Hardware Sync for Jetson]])

### EBIMU 수신기 UART

- JST-GH 4pin (TX, RX, GND, **5V** — EBIMU 수신기 전원)

## 핀맵 (MCU 측, H743)

| 신호 | MCU 핀 | 상태 |
|---|---|---|
| FDCAN1_TX | PD1 | ✅ legacy W12 |
| FDCAN1_RX | PD0 | ✅ legacy W12 |
| UART2_TX (Jetson) | PA2 | 🚩 CubeMX 확정 필요 |
| UART2_RX (Jetson) | PA3 | 🚩 CubeMX 확정 필요 |
| SYNC_OUT | PB14 | 🚩 CubeMX 확정 필요 (TIM 가능 핀) |
| SYNC_IN | PB15 | 🚩 CubeMX 확정 필요 (EXTI 가능 핀) |
| EBIMU UART_TX | (SPARE) | 🚩 SDMMC1 D2/D3 충돌 피해 배정 |
| EBIMU UART_RX | (SPARE) | 🚩 동일 |

## 회로 (CAN 격리 체인)

```
Elmo 드라이버 (모터 쪽)
       │
 CAN 커넥터 (JST-GH 5pin) ← 3mm
       │
 SM712-02HTG TVS (500W, ±8kV ESD) ← 3mm
       │
 ACM2012-900 CMC ← 8mm
       │
 [22Ω × 2] 버스 댐핑 ← 13mm
       │
 ISO1050 버스측 (VCC2 격리 5V) ← 18mm
       │
       │  ═══ 갈바닉 격리 경계 ═══
       │  (5000 Vrms, creepage 6mm)
       │
 ISO1050 MCU측 (VCC1 3.3V AGND) ← 18mm
       │
 PRTR5V0U2X ESD ← 23mm
       │
 [33Ω × 2] MCU 댐핑 ← 28mm
       │
 STM32H743 FDCAN1 TX/RX ← 35mm
```

출처: `_legacy/EXOSUIT_PROTECTION.md` PCB 레이아웃 규칙.

## 관련 개념

- [[Isolated CAN]]
- [[GND Bounce Protection]]
- [[Hardware Sync for Jetson]] — 사용자 확정 (v3.10)

## 관련 부품

- [[ISO1050]] — 격리 CAN 트랜시버
- [[MGJ2D05050SC]] — 격리 DC/DC
- [[TCAN1462]] — 이전 선택 (rejected)

## 데이터시트

- [ ] TI ISO1050 (SLLS983)
- [ ] Murata MGJ2D05050SC
- [ ] TDK ACM2012-900
- [ ] Semtech SM712-02HTG

## 작업 체크리스트

- [ ] ISO1050 심볼·풋프린트 (SOIC-16 wide)
- [ ] MGJ2D05050SC 풋프린트 (SIP-7)
- [ ] 격리 경계 PCB 레이아웃 규칙 문서화 (2mm 갭)
- [ ] 120Ω 종단 점퍼 설계
- [ ] SYNC JST-GH 3pin 배치 (Jetson UART 커넥터 옆)
- [ ] **CubeMX `.ioc` 핀 충돌 검사** (FDCAN1 외 모든 핀)
- [ ] sheet 그리기
- [ ] ERC 통과
