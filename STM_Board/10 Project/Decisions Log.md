---
aliases: [Decisions Log, Decisions]
tags: [project, decisions]
---

# Decisions Log

시간순 기록. 자세한 결정 근거는 [[RECONCILIATION]] 및 각 `[[wikilink]]` 노트 참고.

## 2026-04-17 (내 atomic 노트 세션 초기)

| 항목 | 결정 | 관련 | 상태 |
|---|---|---|---|
| 모터 모델 | 보류 (Phase 5) — Elmo 가정 | [[Open Questions]] | legacy 에선 3 후보 명시 |
| 모터 인터페이스 | CANopen | [[Comms]] | legacy 동일 |
| 로드셀 ADC | HX711 → ADS131M04 | [[Loadcell Amplifier Design]] | **[[RECONCILIATION]] 후 유지** |
| 로깅 | 온보드 microSD SDMMC 4-bit | [[Storage]] | |
| CAN 버스 | 1개 | [[Comms]] | |
| IMU | EBIMU 무선 | [[Goals & Requirements]] | legacy 에선 V5 명시 |
| EtherCAT | 사용 안 함 | | |
| 무선 제어 | ESP32-C3-MINI-1U (IPEX) | [[Wireless]] | 내 추가, 유지 |
| Jetson 통신 | UART + HW SYNC GPIO | [[Hardware Sync for Jetson]] | 내 추가, 유지 |
| E-stop | 이중 안전 (SW + HW AND gate) | [[E-stop Dual Safety]] | 유지 |
| 모터 전원 보드 통과 | 확정 | | |
| 모터 전류 | 15A+ 연속 / 30A 피크 | | legacy 는 계산식 명시 |
| DC 입력 커넥터 | XT60PW (30A) | | Phase B |
| PCB | 6-layer 50×50mm | [[PCB Stackup 6-layer]] | |
| 외/내층 동박 | 2oz / 1oz | | |
| 표면 처리 | ENIG | | |
| 배터리 모니터 | INA228 | [[Battery SoC Estimation]] | **v3.5 에서 × 2 로 변경** |

## 2026-04-20 오전 (Obsidian vault 재구성)

| 항목 | 결정 | 관련 |
|---|---|---|
| Vault 구조 | Obsidian atomic + MOC + wikilink | [[Exosuit Board MOC]] |
| 통합 vault | `~/research-vault/` 단일 + symlink | |
| symlink 이름 | `stm-board` | |

## 2026-04-20 오후 (v3.5 Legacy Reconciliation) ⭐

Legacy 설계 문서 (2026-04-12~14) 발견 → 내 노트와 diff → **주요 결정 교정**.
자세한 근거: [[RECONCILIATION]]

| 항목 | 내 원안 | Legacy | **최종** | 변경 이유 |
|---|---|---|---|---|
| **MCU** | H723VGT6 | H743VIT6 | [[STM32H743VIT6]] | Flash 2MB/RAM 1MB/dual-bank/Nucleo 레퍼런스 |
| **제어 루프** | 1 kHz | 500 Hz | **500 Hz** | AR_Walker 기존값, Elmo+로드셀 충분 |
| **CAN 트랜시버** | TCAN1462 | ISO1050 | [[ISO1050]] | **GND bounce 2.5V 로 이전 보드 파손** |
| **격리 DC/DC** | 없음 | MGJ2D05050SC | [[MGJ2D05050SC]] | ISO1050 VCC2 독립 공급 |
| **로드셀 ADC** | ADS131M04 | ADS1234 + INA128UA | **ADS131M04 유지** | 32 kSPS 동시 샘플링·PGA 내장·단일 3.3V·비용 절반 |
| **배터리 모니터** | INA228 × 1 | INA228 × 2 | INA228 × 2 | 24V×2 직렬 시 개별 측정 |
| **배터리** | 24~48V 미정 | 6S Li-ion 25.2V | 6S 25.2V 확정 | legacy 명시 |
| **IMU** | 일반 EBIMU | EBIMU V5 (2.4GHz) | EBIMU V5 | |
| **카메라** | 언급 없음 | ZED X Mini (GMSL2) | ZED X Mini | Jetson 전용 |
| **Jetson** | 일반 Orin | Orin NX 16GB | Orin NX 16GB | 157 TOPS, ZED Link |
| **모터 제어 모드** | MIT 토크 | Servo + SET_POS | Servo + SET_POS | 풀리 r(θ) 가변 → 어드미턴스로 보정 |

### v3.5 신규 개념 추가 (legacy 보호 아키텍처)

| 개념 | 문서 | 문제 해결 |
|---|---|---|
| [[Regen Energy Protection]] | 신규 | 4 모터 동시 릴리즈 1J → 143V 폭주 방지 |
| [[GND Bounce Protection]] | 신규 | 2.5V bounce → CAN TXD/RXD 파손 근본 해결 |
| [[Isolated CAN]] | 신규 | ISO1050 + MGJ2D05050SC 격리 체인 |
| [[Brake Resistor Circuit]] | 신규 | 26.5V 임계 → 3Ω/50W 소산 |
| [[Inrush Current Limiting]] | 신규 | NTC 4Ω + G5V-1 bypass 릴레이 |

### v3.5 회로 필수 수정 (legacy C4~C7, W2~W7)

| 항목 | 수정 |
|---|---|
| C4 VCAP_2 (Pin 57) | 2.2µF 독립 배치 ([[MCU Core]]) |
| C5 VREF+ | VDDA 직결 + 100nF + 1µF |
| W2 VDDA 필터 | BLM18PG601 ferrite + 10µF+1µF+100nF |
| W4 HSE 크리스털 | OSC_IN 22Ω 직렬 저항 |
| W6 BOOT0 | 10kΩ 풀다운 + 점퍼 |
| W7 NRST | 100nF |
| W11 CAN STBY | ISO1050 로 변경돼 N/A |

## 템플릿

공용 템플릿 (Component / Concept / Decision) 은 `~/research-vault/templates/` 에, 프로젝트 전용 ([[Schematic Block]]) 은 `90 Templates/`.
