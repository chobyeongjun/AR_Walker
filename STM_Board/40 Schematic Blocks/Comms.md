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
- **격리 CAN-B** (Elmo 드라이버 CANopen) — [[ISO1050]] + [[MGJ2D05050SC]]
- **Jetson UART + HW SYNC** (pose 다운링크)
- **EBIMU UART** (무선 IMU 수신기 입력)

→ [[Wireless]] (ESP32) 는 별도 블록

## ⚠️ 격리 CAN (legacy review 핵심 변경)

내 원안 [[TCAN1462]] (비격리) → **[[ISO1050]] (격리)** 로 변경.

이유: [[GND Bounce Protection]] · [[Isolated CAN]] · [[RECONCILIATION]]
요약: 이전 보드가 GND bounce 2.5V 로 CAN TXD/RXD 반복 파손 → 격리 필수.

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

### Jetson UART + SYNC

- JST-GH 4pin (TX, RX, GND, 3V3)
- JST-GH 3pin (SYNC_OUT, SYNC_IN, GND)

### EBIMU 수신기 UART

- JST-GH 4pin (TX, RX, GND, **5V** — EBIMU 수신기 전원)

## 핀맵 (MCU 측, H743)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| FDCAN1_TX | PD1 | → ISO1050 TXD (33Ω) |
| FDCAN1_RX | PD0 | ← ISO1050 RXD (33Ω) |
| UART2_TX (Jetson) | PA2 | |
| UART2_RX (Jetson) | PA3 | |
| SYNC_OUT | PB14 | TIM PWM 가능 |
| SYNC_IN | PB15 | EXTI |
| EBIMU UART_TX | (SPARE) | SDMMC1 충돌 피해 배정 |
| EBIMU UART_RX | (SPARE) | |

## 회로 (CAN 격리 체인, 커넥터→MCU 방향)

```
Elmo 드라이버 (모터 쪽)
       │
 CAN 커넥터 (JST-GH 5pin) ← 3mm
       │
 SM712-02HTG TVS (500W, ±8kV ESD) ← 3mm
       │
 ACM2012-900 CMC (CM 노이즈 차단) ← 8mm
       │
 [22Ω × 2] 버스 댐핑 ← 13mm
       │
 ISO1050 버스측 (VCC2 = 격리 5V) ← 18mm
       │
       │  ═══ 갈바닉 격리 경계 ═══
       │  (5000 Vrms, creepage 6mm)
       │
 ISO1050 MCU측 (VCC1 = 3.3V AGND) ← 18mm
       │
 PRTR5V0U2X ESD ← 23mm (MCU 측)
       │
 [33Ω × 2] MCU 댐핑 ← 28mm
       │
 STM32H743 FDCAN1 TX/RX ← 35mm
```

## 관련 개념

- [[Isolated CAN]]
- [[GND Bounce Protection]]
- [[Hardware Sync for Jetson]]

## 관련 부품

- [[ISO1050]] — 격리 CAN 트랜시버
- [[MGJ2D05050SC]] — 격리 DC/DC
- [[TCAN1462]] — 이전 선택 (rejected)

## 데이터시트

- [ ] TI ISO1050 데이터시트
- [ ] Murata MGJ2D05050SC 데이터시트
- [ ] TDK ACM2012-900 데이터시트
- [ ] Semtech SM712-02HTG 데이터시트

## 작업 체크리스트

- [ ] ISO1050 심볼·풋프린트 (SOIC-16 wide)
- [ ] MGJ2D05050SC 풋프린트 (SIP-7)
- [ ] 격리 경계 PCB 레이아웃 규칙 문서화 (2mm 갭)
- [ ] 120Ω 종단 점퍼 설계
- [ ] 소자 배치 거리 (커넥터→MCU 35mm 내외) 설계
- [ ] sheet 그리기
- [ ] ERC 통과
