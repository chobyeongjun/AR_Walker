---
aliases: [Components]
tags: [moc, components]
---

# Components MOC

부품별 atomic 노트 인덱스. 신규 추가는 v3.5 (legacy reconciliation) 반영.

## Phase A — 메인 부품

### MCU & 주변
- [[STM32H743VIT6]] ⭐ 메인 MCU (H723 에서 변경, [[RECONCILIATION]])
- [[ESP32-C3-MINI-1U]] — BLE/WiFi 무선 (IPEX 외부 안테나)

### 센서 / ADC
- [[ADS131M04]] — 24-bit 로드셀 ADC (32 kSPS, 동시 샘플링)
- [[REF5025]] — 2.5V 정밀 레퍼런스
- [[AS5048A]] — SPI 자기식 인코더 × 2
- [[INA228]] — 배터리 모니터 **× 2** (직렬 팩 개별)

### 통신 (격리)
- [[ISO1050]] ⭐ 격리 CAN 트랜시버 ([[TCAN1462]] 에서 변경)
- [[MGJ2D05050SC]] — 격리 DC/DC (VCC2 공급)
- [[TPD4S012]] — USB ESD
- [[TCAN1462]] — ❌ rejected (비격리, GND bounce 취약)

### 전원·보호 (기본)
- [[TPS62933]] — 2.5MHz 60V buck → 5V
- [[TLV75533]] — 3.3V LDO × 2 (디지털/아날로그)

### 안전·UI
- [[74LVC1G08]] — AND gate (E-stop HW)

### 저장
- [[DM3AT microSD]] — push-push 슬롯 SDMMC 4-bit

## Phase B — 모터 전원 보호 (legacy 반영)

### 역전압 + eFuse 경로
- [[LTC4368-1]] — eFuse 컨트롤러 (내 원안; legacy 는 discrete)
- [[IPB180N06S4]] — eFuse N-MOSFET
- **또는 legacy 방식 (discrete)**:
  - `DMP2305U-7` — 역전압 P-MOS (SOT-23)
  - `BZT52C12` — P-MOS GATE 12V 제너 클램프

### 인러시 제한
- `SL08-2R005` — NTC 2Ω × 2 직렬
- `Omron G5V-1-DC5` — NTC bypass 릴레이

### 버스 보호
- `AP63205WU-7` — 벅 컨버터 (legacy C1 — AP62200WU 에서 변경, 32V OK)
- `P6KE33CA` — 버스 TVS 600W × 2 병렬 (37.8V 클램프)
- `Rubycon 35ZLH4700M` — 벌크 캡 4700µF/35V × 2 (11,280µF 합)

### 모터 로컬 보호 (× 4)
- `SMAJ28CA` — 모터 TVS 400W
- `Panasonic SVPD471M35` — OS-CON 470µF/35V
- `Murata BLM31AG601SN1L` — 페라이트 비드 600Ω/3A

### 브레이크 (재생 에너지 소산)
- `TLV3201` — 단전원 비교기 (26.5V 임계)
- `IPB025N10N3` — 브레이크 MOSFET (100V/130A)
- `Arcol HS50 3R0` — 브레이크 저항 3Ω/50W

### GND 연결
- `BLM21PG601SN1D` — Star ground 페라이트 (PGND ↔ AGND)

## Phase C — 시스템 스펙 (legacy 추가)

- `STM32H743VIT6` (위)
- `EBIMU EBMotion V5` — 무선 IMU (수신기 UART)
- `ZED X Mini` — Stereolabs 카메라 (GMSL2, Jetson 전용)
- `Jetson Orin NX 16GB` — 고수준 컴퓨팅 (157 TOPS)

## 보류 (Phase 5 — 모터 확정 후)

- **모터**: T-Motor U8 Lite KV85 / CubeMars RI60 KV120 / Maxon EC-i 40
- **Elmo 드라이버**: Gold Twitter / Solo Twitter / Whistle

→ [[Open Questions]] · [[Modular Phase A-B Strategy]]
