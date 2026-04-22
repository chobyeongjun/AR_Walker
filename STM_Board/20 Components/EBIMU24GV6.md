---
aliases: [EBIMU, EBMotion V5, 무선 IMU]
tags: [type/component, domain/sensor, phase/A, status/decided]
type: component
part_number: EBIMU EBMotion V5
package: module + 2.4GHz RF receiver
manufacturer: E2BOX
voltage_range: "sensor 3.3V, receiver 5V"
interface: UART (receiver side)
phase: A
status: decided
---

# EBIMU EBMotion V5

## 한 줄 요약

2.4GHz 무선 IMU — 1000Hz 샘플링, Quaternion/Euler 출력. 수신기 UART 로 MCU 직결.

## 사양

- 센서: 9-DOF IMU (accel + gyro + mag)
- 샘플링: **1000 Hz**
- 출력 포맷: Quaternion, Euler, Raw
- 무선: 2.4GHz 전용 RF (Wi-Fi 와 다름)
- 수신기: USB 또는 UART
- 전원 (센서 측): 3.3V
- 전원 (수신기 측): 5V
- 도달 거리: ~10m (실내 LOS)

## 왜 선택했나

- **wearable 완전 무선** — 케이블 간섭 없음
- 1kHz 샘플링 — 500Hz 제어 루프 + 2× 오버샘플링 여유
- Quaternion 직접 제공 — MCU 에서 센서 fusion 부담 ↓
- 전용 RF → Wi-Fi/BLE 간섭 없음

## 연결 (보드 측)

```
EBIMU 수신기 (UART 모드)
      ↓ 4-wire UART + power
JST-GH 4pin 커넥터 (보드 상)
  [TX, RX, GND, 5V]
      ↓
STM32H743 UART (EBIMU 전용)
```

→ [[Comms]] 블록 내 JST-GH 4pin (TX, RX, GND, **5V 공급**)

**주의:** EBIMU 수신기는 **5V** 입력 — 3.3V UART 만 있는 보드에선 별도 5V 출력 라인 필요.

## 관련 블록

- [[Comms]] — UART 연결

## 데이터시트

- [ ] E2BOX EBIMU EBMotion V5 매뉴얼 PDF
- [ ] UART 프로토콜 스펙 (binary frame 포맷)
