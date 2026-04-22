---
aliases: [EBIMU V6, EBIMU24GV6, EBRCV24GV6, EBIMU EBMotion V5, EBIMU V5, EBIMU, 무선 IMU]
tags: [type/component, domain/sensor, phase/A, status/decided]
type: component
part_number: EBIMU24GV6 (IMU) + EBRCV24GV6 (수신기)
package: wireless sensor module + RF receiver dongle
manufacturer: E2BOX
voltage_range: "IMU 3.5-6.0V USB, 수신기 4.5-6.0V USB"
interface: 2.4GHz RF → 수신기 UART (STM32 측)
phase: A
status: decided
---

# EBIMU24GV6 + EBRCV24GV6

> **✅ 사용자 확정 V6 모델** (legacy HANDOFF V5 는 오기, 실제 V6).
> **모든 사양 아래는 공식 E2BOX 데이터시트 기반** ([[_legacy/RECONCILIATION|reconciliation]] 이후 검증 완료):
> - `50 References/Datasheets/17_System_Modules/EBIMU24GV6_IMU_Module.pdf` (34 pages, Rev1.0, F/W v610)
> - `50 References/Datasheets/17_System_Modules/EBRCV24GV6_Receiver.pdf` (29 pages, Rev1.0, F/W v6.00)

## 한 줄 요약

2.4GHz 무선 AHRS (Attitude/Heading Reference System). 웨어러블 센서 `EBIMU24GV6` 가 RF 로 `EBRCV24GV6` 수신기에 전송 → 수신기 UART (최대 921600 bps) 가 STM32 로 직결.

## 구성

```
┌────────────────────────┐        2.4GHz RF         ┌───────────────────────┐     UART 3.3V TTL       ┌──────────┐
│ EBIMU24GV6 (IMU 센서)  │  ─────────────────────►  │ EBRCV24GV6 (수신기)   │  ────────────────────►  │ STM32H743│
│ 24.6 × 32.0 × 5.3 mm   │     2400~2525 MHz         │ 39.0 × 26.0 mm        │     up to 921600 bps    │ UART_RX  │
│ 3.3V USB-C + 1S LiPo   │     2000 kbps, +4 dBm     │ USB-C + SMA 안테나    │     TX: HIGH≥2.2V       │          │
│ Operating 30 mA        │     125 채널, 100 ID       │ Operating 45 mA       │         LOW≤0.4V        │          │
└────────────────────────┘                           └───────────────────────┘                         └──────────┘
```

## EBIMU24GV6 (IMU 센서) 상세

### Attitude / Heading 정확도 (§8-1)

| 항목 | 값 |
|---|---|
| Static accuracy (roll/pitch) | **< 0.1°** (magneto ON) / < 0.02° (magneto OFF) |
| Static accuracy (yaw) | < 0.5° |
| Dynamic accuracy (RMS) | < 1.5° (magneto ON) / < 0.5° (magneto OFF) |
| Angular resolution | **0.01°** |
| Range | roll ±180°, pitch ±90°, yaw ±180° |
| **내부 자세 갱신속도** | **8000 Hz** ← 500 Hz 제어 루프 대비 16× 오버샘플링 |
| 출력 데이터 속도 | **1 Hz ~ 1000 Hz** (수신기 `sor` 명령으로 설정) |

### 센서 사양 (§8-2, 8-3, 8-4)

| 센서 | 범위 | 감도 | 대역폭 |
|---|---|---|---|
| Gyroscope | ±125 ~ ±2000 dps | 3.8~61.0 mdps | 1000 Hz |
| Accelerometer | ±16 g | 0.061~0.488 mg | 1000 Hz |
| Magnetometer | ±3000 μT | 0.097 μT | 100 Hz |

### 전원 (§7-2)

| 항목 | 값 |
|---|---|
| USB 공급 전압 | 3.5 V (min) / **5.0 V (typ)** / 6.0 V (max) |
| 절대최대 | -0.3 ~ +6.5 V |
| 운용 전류 | **30 mA** (Normal) |
| 충전 전류 (CC) | 196 mA |
| 충전 종료 전압 (CV) | 4.2 V |
| 배터리 수명 (240 mAh) | ~ 8 시간 |

**배터리 커넥터:** Molex 53261-02 (= 51021-02 호환). 1셀 LiPo 내장 충전회로.

### RF 사양 (§7-3)

| 항목 | 값 |
|---|---|
| 주파수 | 2400 ~ 2525 MHz |
| 주파수 편차 | ±320 kHz |
| Air Data Rate | 2000 kbps |
| 채널 간격 | 1 MHz (총 126 채널, 0~125) |
| ID 범위 | 0~99 (채널당 최대 100) |
| 출력 파워 | +4 dBm |

### 기계 (§9)

| 항목 | 값 |
|---|---|
| **치수** | **24.6 × 32.0 × 5.3 mm** |
| 온도 (보관) | -40 ~ +85°C |
| 온도 (동작) | -10 ~ +75°C |
| 충격 (전원 OFF) | 10000g / 0.2ms, 2000g / 1.0ms |
| Free fall | 1.8 m |
| ESD | ±2 kV HBM / ±200 V CDM |

## EBRCV24GV6 (수신기) 상세

### UART 출력 (§5-1-1, §6-2)

| 항목 | 값 |
|---|---|
| Baudrate | 9600/19200/38400/57600/115200/230400/460800/**921600** bps |
| 기본값 | **921600 bps** (sb=8) |
| 프레임 | 8-N-1, flow control 없음 |
| TX output HIGH | ≥ 2.2 V (3.3V TTL) |
| TX output LOW | ≤ 0.4 V |
| 3V 출력 전류 | 100 mA |

**⚠️ 수신기 외부 헤더 핀: 5V · GND · 3V (출력) · TX** (datasheet §2-1 PCB 실사 기반)
→ **RX 없음. 커맨드 설정은 USB-C 로만 가능.** (PC 또는 Jetson 에서 최초 설정 후 보드에 마운트)

### 출력 데이터 속도 (§5-1-2)

| 센서 수 | 최대 속도 |
|---|---|
| 1개 | **1000 Hz** (sor=1) |
| 15개 | **85 Hz** (실시간 처리 한계) |
| 기본값 | 62.5 Hz (sor=16) |

→ **우리 1-센서 구성 + 500Hz 제어 루프** = sor=2 (500Hz) 로 설정하면 정합.

### 출력 포맷 (§4-1, §4-2)

- **ASCII 모드** (soc=1, default): 사람이 읽기 쉬움, CSV-like. `100-0,-0.2686,0.0945,...,50\r\n`
- **HEX binary 모드** (soc=2): 효율적. 2 byte/item + checksum
- 자세 포맷: Euler (Roll/Pitch/Yaw) 또는 Quaternion (z/y/x/w) 중 택일
- 최대 15 데이터 항목 동시 출력 (gyro/accel/mag/dist/temp/batt/timestamp)

### 전원 (§6-2)

| 항목 | 값 |
|---|---|
| USB 공급 전압 | 4.5 / **5.0** / 6.0 V |
| 운용 전류 | **45 mA** |
| 온도 (동작) | -10 ~ +70°C |

### 기계 (§7)

| 항목 | 값 |
|---|---|
| **치수** | **39.0 × 26.0 mm** |
| 마운팅 홀 | 4× Φ 3.1 mm, 모서리 3mm 이격 |
| RF 안테나 | SMA 커넥터 (50Ω), DIPOLE MALE |

## 보드 연결 설계

### 인터페이스 선택: UART 헤더 방식 ⭐

```
EBRCV24GV6 수신기 (PC/Jetson 에 USB-C 초기 연결해서 ch/id/baud 설정 후)
     │
 TX (3.3V TTL, 921600 bps) ─ GND ─ 5V 공급
     │
 JST-GH 3pin 커넥터 (보드 상)
   [TX,  GND,  5V]       ← 3V 핀 미연결 (수신기 내장 LDO, 우리 불필요)
     │
 STM32H743 UART RX 핀 (🚩 CubeMX 확정)
```

**⚠️ JST-GH 핀 수 정정:** 기존 BOM "4pin (TX/RX/GND/5V)" 는 오기 — 수신기에 **RX 헤더 없음**. **3pin (TX/GND/5V)** 으로 충분.

### 초기 설정 워크플로

1. 수신기를 PC 에 USB-C 로 연결 (시리얼 터미널: 921600 bps, 8-N-1)
2. `<pair>` 로 센서 페어링 (자동 ch/id 설정)
3. `<sor2>` 로 500Hz 출력 설정
4. `<soc2>` 로 HEX binary 모드 (MCU 파싱 효율 ↑)
5. `<sob0>` 로 배터리 잔량 출력 OFF (데이터 절약, 필요하면 유지)
6. 수신기를 보드 JST-GH 에 연결 → 자동 스트리밍 시작 (`<pons1>` default)

### STM32 파싱 예시 (HEX 모드)

```
SOP(2)   CH(1)  ID(1)   DATA n × 2byte    CHK(2)
0x5555   0x64   0x00    Roll/Pitch/Yaw    Σ(byte) & 0xFF
         100    0       각 2byte signed    checksum
```

ASCII 기준: `CR/LF` 로 프레임 구분, `,` 로 필드 구분. DMA idle 인터럽트 권장.

## 보드 요구 사항

| 항목 | 값 |
|---|---|
| 커넥터 | JST-GH 3pin (TX/GND/5V) |
| 5V 공급 | TPS62933 → 5V 레일 (이미 존재) |
| UART baud | 921600 bps |
| CubeMX | UART RX only (RX 하나만 활성화) |
| 수신기 물리 위치 | 보드 옆 고정 (SMA 안테나 노출 필요) 또는 케이블로 이격 배치 |

## 관련 블록

- [[Comms]] — UART 연결, 수신기 JST-GH 커넥터
- [[STM32H743VIT6]] — UART RX 핀 (CubeMX 🚩)

## 관련 개념

- SET 900 MHz vs 2.4GHz: E2BOX 는 2.4GHz ISM (Wi-Fi·BLE 와 동일 대역) — 채널 간섭 관리 필요 ([[ESP32-C3-MINI-1U]] 와 채널 충돌 가능)

## 데이터시트 (저장 완료)

- [x] `EBIMU24GV6_IMU_Module.pdf` (662 KB, 34p)
- [x] `EBRCV24GV6_Receiver.pdf` (507 KB, 29p)

## 변경 이력

- Legacy HANDOFF (2026-04-14): "EBIMU EBMotion V5" 표기 (오기)
- 2026-04-22 v3.12: 사용자 확정 V6 모델로 정정, 잠정 스펙 작성
- **2026-04-22 v3.13: 데이터시트 직접 읽어 전 사양 검증, UART 3pin 으로 정정 (기존 4pin 오기), 모든 값 페이지 번호 인용**

## 관련

- [[Comms]]
- [[Citations & Sources]] — E2BOX 데이터시트 1차 출처
- [[_legacy/HANDOFF]] — 원본 V5 표기
- [[_legacy/RECONCILIATION]]
