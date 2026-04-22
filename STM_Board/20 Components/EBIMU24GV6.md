---
aliases: [EBIMU V6, EBIMU24GV6, EBRCV24GV6, EBIMU EBMotion V5, EBIMU V5, EBIMU, 무선 IMU]
tags: [type/component, domain/sensor, phase/A, status/decided]
type: component
part_number: EBIMU24GV6 (IMU) + EBRCV24GV6 (수신기)
package: wireless sensor module + RF receiver dongle
manufacturer: E2BOX
voltage_range: "IMU 3.3V, 수신기 5V"
interface: 2.4GHz RF → 수신기 UART (STM32 측)
phase: A
status: decided
---

# EBIMU24GV6 + EBRCV24GV6

> **✅ 사용자 확정 V6 모델** (legacy HANDOFF 은 V5 로 표기했으나 실제 사용 모델은 V6).
> 2 파트 세트: IMU 센서 (`EBIMU24GV6`) + RF 수신기 (`EBRCV24GV6`).
> 출처: 사용자 로컬 PDF `EBIMU24GV6_rev10.pdf`, `EBRCV24GV6_rev10.pdf` (E2BOX 공식)

## 한 줄 요약

2.4GHz 무선 IMU 시스템. 사용자 몸에 부착된 `EBIMU24GV6` 센서가 `EBRCV24GV6` 수신기로 전송 → 수신기 UART 가 STM32 에 직결.

## 구성

```
[EBIMU24GV6]                      [EBRCV24GV6]                [STM32H743]
 IMU 센서 모듈          ── 2.4GHz ──►  RF 수신기   ── UART ──►  MCU
 (몸에 부착)                        (보드 옆 고정)           (UART Rx)
 3.3V                                5V 입력
 9-DOF                               UART 출력
```

## 사양

> ⚠️ 아래 값은 **E2BOX 시리즈 일반 스펙 및 legacy HANDOFF 기반**. 실제 `EBIMU24GV6_rev10.pdf` / `EBRCV24GV6_rev10.pdf` 의 최신 수치로 사용자가 교정해 주기 바람.

### EBIMU24GV6 (센서 모듈)

| 항목 | 값 (잠정) |
|---|---|
| 센서 | 9-DOF (accelerometer + gyroscope + magnetometer) |
| 샘플링 | 최대 1000 Hz |
| 출력 포맷 | Quaternion / Euler / Raw |
| 무선 | 2.4GHz 전용 RF (Wi-Fi 와 다른 대역) |
| 전원 | 3.3V |
| 용도 | 웨어러블 동작 측정 |

### EBRCV24GV6 (수신기)

| 항목 | 값 (잠정) |
|---|---|
| 입력 | 2.4GHz RF (EBIMU24GV6 로부터) |
| 출력 | UART (TTL 3.3V 또는 5V 레벨) + USB (호스트용) |
| 전원 | **5V 입력** |
| 도달 거리 | ~10m 실내 LOS |

## 왜 선택

- **wearable 완전 무선** — 케이블 간섭 없음
- 1kHz 샘플링 — 500Hz 제어 루프 + 2× 오버샘플링 여유
- Quaternion 직접 제공 — MCU 센서 fusion 부담 ↓
- 전용 RF → Wi-Fi/BLE 간섭 없음

## 보드 측 연결 (수신기 → MCU)

```
EBRCV24GV6 수신기 (UART 모드)
      ↓ 4-wire UART + 전원
JST-GH 4pin 커넥터 (보드 상)
  [TX, RX, GND, 5V]
      ↓
STM32H743 UART (EBIMU 전용, 🚩 CubeMX 핀 확정 필요)
```

→ [[Comms]] 블록 내 JST-GH 4pin (TX, RX, GND, **5V 공급**)

**주의:** EBRCV 는 5V 입력 — 보드 5V 레일 (Buck 출력 또는 별도) 경유 필요.

## 관련 블록

- [[Comms]] — UART 연결
- [[STM32H743VIT6]] — UART 핀 (CubeMX 확정 필요)

## 데이터시트

- [x] `EBIMU24GV6_rev10.pdf` (센서 모듈) — 사용자 `~/Downloads/` → `17_System_Modules/EBIMU24GV6_IMU_Module.pdf` 로 이동 예정
- [x] `EBRCV24GV6_rev10.pdf` (수신기) — 사용자 `~/Downloads/` → `17_System_Modules/EBRCV24GV6_Receiver.pdf` 로 이동 예정
- [ ] UART 프로토콜 스펙 (binary frame 포맷) — PDF 확인 후 정리

## 변경 이력

- **Legacy HANDOFF (2026-04-14)**: "EBIMU EBMotion V5" 표기 — legacy 시기 모델
- **2026-04-22 정정**: 실제 사용 모델은 **V6** (`EBIMU24GV6` + `EBRCV24GV6`).
  Legacy V5 표기는 `_legacy/HANDOFF.md` 원본에 그대로 보존 (역사 기록).
  실제 회로 설계·BOM·Block Diagram 은 V6 기준으로 갱신.

## 관련

- [[Comms]]
- [[Decisions Log]] — V5 → V6 정정 기록
- [[_legacy/HANDOFF]] — 원본 V5 표기 보존
