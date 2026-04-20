---
aliases: [Legacy Reconciliation, v3.5 Reconciliation]
tags: [type/decision, project/reconciliation]
---

# Reconciliation: Legacy vs Current Notes

> 2026-04-14 legacy 설계 문서 (`BOARD_DESIGN_REVIEWED.md`, `EXOSUIT_PROTECTION.md`, `HANDOFF.md`) 와 2026-04-17~20 에 작성한 내 atomic 노트 간 차이를 정리하고 최종 결정을 고정.

## 🔴 결정 교정 (legacy 가 정답 → 내 노트 수정)

| 항목 | 내 원안 | Legacy | **최종** | 이유 |
|---|---|---|---|---|
| **MCU** | STM32H723VGT6 | STM32H743VIT6 | **H743VIT6** | Flash 2MB / RAM 1MB / dual-bank / Nucleo-H743ZI / ST HAL 성숙도 → H723 의 USB HS 내장 장점은 우리 use case 에 무의미 |
| **제어 루프** | 1 kHz | 500 Hz | **500 Hz** | AR_Walker 기존 값 유지, Elmo + 로드셀 샘플링에 충분 |
| **CAN 트랜시버** | TCAN1462 (비격리) | ISO1050 (격리) | **ISO1050** | 이전 보드 GND bounce 2.5V 로 TXD/RXD (±0.3V 절대최대) 파손 — 격리 필수 |
| **배터리 모니터 수** | INA228 × 1 | INA228 × **2** | **INA228 × 2** | 24V×2 직렬 시 셀 팩 개별 측정 |
| **배터리 팩** | 24~48V 미정 | 6S Li-ion 25.2V (+ 옵션 48V 직렬) | **6S Li-ion 25.2V 확정, 48V 직렬은 Phase B** | legacy 명시 |
| **IMU 모델** | 일반 EBIMU | EBIMU EBMotion V5 (2.4GHz RF) | **EBIMU V5** | 1000Hz, Quaternion/Euler |
| **카메라** | 언급 없음 | ZED X Mini (GMSL2, IP67, 150g) | **ZED X Mini** | Jetson 전용 |
| **Jetson 모델** | 일반 Orin | Orin NX 16GB (157 TOPS) | **Orin NX 16GB** | ZED Link PCIe 필요 |
| **모터 제어 모드** | MIT 토크 | Servo(전류) + SET_POS | **Servo + SET_POS** | 케이블 드리븐 풀리 반경 r(θ) 가변 → T_ff 계산 어려움, 어드미턴스가 로드셀 피드백으로 보정 |

## ✅ 내 원안 유지 (재검토 후)

| 항목 | 내 선택 | Legacy | 판단 |
|---|---|---|---|
| **로드셀 ADC** | ADS131M04 | ADS1234 + INA128UA | **ADS131M04 유지** — 32 kSPS 동시 샘플링, PGA 내장, 3.3V 단일, 부품 수·면적·비용 우위 |
| **6-layer 50×50mm** | 확정 | 명시 없음 | 유지 |
| **JST-GH 1.25mm** | 확정 | XH 혼재 | JST-GH 유지 (컴팩트) |
| **Hardware SYNC GPIO** | 추가 | 없음 | 유지 — Jetson pose 타임 정합성 |
| **ESP32-C3-MINI-1U** | 추가 | 없음 | 유지 — BLE 5.0 + WiFi, IPEX 외부 안테나 |
| **SoC 쿨롱 카운팅** | INA228 CHARGE 레지스터 활용 | 하드웨어 측정만 | 유지 — SW 알고리즘 추가 가치 |

## 🚨 내가 완전히 놓친 것 (legacy 추가) — 전부 채택

### 보호 아키텍처 (이전 보드 파손의 근본 원인)

1. **재생 에너지 보호** — 4 모터 동시 릴리즈 1J → V_peak = 143V (100µF) 시 즉사
   → 11,280µF 벌크 + **브레이크 저항 회로** (26.5V 임계)
2. **GND bounce 보호** — 50A/µs × 50nH = 2.5V bounce → CAN TXD/RXD ±0.3V max 초과
   → **ISO1050 격리 (5000 Vrms)** + 격리 DC/DC
3. **인러시 제한** — 2× SL08-2R005 NTC 직렬 (4Ω) + Omron G5V-1 bypass 릴레이
4. **모터별 로컬 보호** — SMAJ28CA + 470µF/35V OS-CON + BLM31AG × 4
5. **버스 TVS** — P6KE33CA × 2 병렬 (37.8V 클램프, AK 모터 내압 40V 이내)
6. **P-MOS VGS 보호** — GATE-GND BZT52C12 12V 제너 + R 100kΩ (절대최대 ±20V 이내)

### MCU 회로 critical 누락 (legacy review C4~C7, W1~W15)

| # | 항목 | 수정 |
|---|---|---|
| C4 | VCAP_2 핀 (Pin 57) | 2.2µF X5R 독립 배치 |
| C5 | VREF+ 핀 (Pin 21) | VDDA 직결 + 100nF + 1µF |
| W2 | VDDA 필터 | 3.3V → BLM18PG601 600Ω@100MHz → VDDA, 10µF+1µF+100nF |
| W4 | HSE 크리스털 | OSC_IN 에 **22Ω 직렬 저항** (고조파 억제) |
| W5 | USB-C CC1/CC2 | 각각 5.1 kΩ → GND + PRTR5V0U2X ESD |
| W6 | BOOT0 | 10 kΩ 풀다운 + 점퍼 |
| W7 | NRST | 100nF → GND |
| W11 | CAN STBY | GND 직결 또는 10k 풀다운 |
| W9/W10 | 로드셀 입력 | 10kΩ + BAV99 클램프 + 1kΩ + 150nF 안티앨리어싱 |

## 📋 적용 위치 (atomic 노트)

| 변경 | 적용 대상 |
|---|---|
| MCU H743VIT6 | `20 Components/STM32H743VIT6.md` (신규), `STM32H723VGT6.md` 삭제 |
| MCU 회로 수정 | `40 Schematic Blocks/MCU Core.md` 업데이트 |
| ISO1050 격리 CAN | `20 Components/ISO1050.md` (신규), `TCAN1462.md` 보관/deprecated, `Comms.md` 업데이트 |
| 재생에너지 보호 | `30 Concepts/Regen Energy Protection.md` (신규) |
| GND bounce | `30 Concepts/GND Bounce Protection.md` (신규) |
| 브레이크 저항 | `30 Concepts/Brake Resistor Circuit.md` (신규) |
| 인러시 제한 | `30 Concepts/Inrush Current Limiting.md` (신규) |
| 격리 CAN | `30 Concepts/Isolated CAN.md` (신규) |
| 모터 보호 부품 | `20 Components/` 에 신규 노트 다수 (P6KE33CA, SMAJ28CA, DMP2305U, BZT52C12, …) |
| Motor Power 블록 | `40 Schematic Blocks/Motor Power.md` 전면 rewrite |
| 시스템 스펙 | `Goals & Requirements.md`, `Block Diagram.md` 업데이트 |
| 배터리 모니터 × 2 | `Battery Monitor.md` 블록 업데이트 |

## 🎯 여전히 보류 (legacy 도 미결, Phase B 에서 확정)

- **모터 모델**: T-Motor U8 Lite KV85 / CubeMars RI60 KV120 / Maxon EC-i 40 (3 후보)
- **Elmo 드라이버**: Gold Twitter / Solo Twitter / Whistle (3 후보)
- **배터리 최종 전압**: 24V 단일 vs 48V 직렬 (모터 확정 후)
- **필요 연속 전류 계산** (legacy 에 공식 명시):
  ```
  F_cable    = τ_assist / d_moment
  τ_motor    = F_cable × r_pulley
  I_required = τ_motor / Kt
  ```
  → 보행 데이터 (정상인 70kg) + 보조 비율 (10~30%) → 토크 → 전류 역산

## 📅 진행 이력

- 2026-04-12~14 : legacy 설계 + 4-agent review (사용자 + 이전 Claude 세션)
- 2026-04-17~20 : 내 atomic 노트 작성 (legacy 모름 상태)
- 2026-04-20 : legacy 발견 → 이 reconciliation 작성 → **H743 + ADS131M04 + ISO1050 + 보호 아키텍처 전체 채택**
- 다음: [[Phase Plan]] P1 데이터시트 수집 → P2 KiCad 프로젝트
