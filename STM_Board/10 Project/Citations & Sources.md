---
aliases: [Citations, Sources, Provenance]
tags: [project, references]
---

# Citations & Sources

> 이 vault 의 각 주장·스펙이 어디서 왔는지 투명하게 기록.
> 내가 이 세션에서 생성한 atomic 노트의 **정확성과 책임 소재** 를 명확히 하기 위함.

## 📌 일반 원칙

1. **벤더 데이터시트 = 1차 출처** (ST, TI, Vishay 등)
2. **legacy 문서 = 검증된 2차 출처** (4-agent 리뷰 거친 값)
3. **내 엔지니어링 판단 = 3차** (표준 관행, 데이터시트 읽은 뒤 합리적 추론)
4. **확인 안 된 값 = 회색 영역** (내가 모르면서 추측한 거, 반드시 표시)

## 📚 Primary Sources (이 프로젝트에서 사용된 것)

### A. Legacy 문서 (2026-04-12~14 Claude Code 세션 산출물)

`10 Project/_legacy/` 에 원본 보존. **내 v3.5 reconciliation 이후 많은 스펙의 정답 source**.

| 파일 | 제공하는 스펙 |
|---|---|
| `HANDOFF.md` | MCU=H743, IMU=EBIMU V5, 카메라=ZED X Mini, Jetson=Orin NX 16GB, 제어=500Hz 어드미턴스, 모터 모드=Servo+SET_POS |
| `BOARD_DESIGN_REVIEWED.md` | **CRITICAL C1~C7, WARNING W1~W15** — STM32H743 핀 번호 (VCAP_2 Pin 57, VREF+ Pin 21), Buck 선정 (AP63205WU), 로드셀 체인 (INA128+ADS1234, 나중에 ADS131M04 로 재검토) |
| `EXOSUIT_PROTECTION.md` | 재생 에너지 1J·143V·11,280µF, GND bounce 2.5V·50A/µs, 브레이크 임계 26.5V, 보호 부품 전체 (P6KE33CA, SMAJ28CA, DMP2305U, IPB025N10N3, TLV3201, SL08-2R005 등) |
| `BOARD_PLAN.md` | Phase 구조, AR_Walker 레포 분석 결과 (Teensy 4.1, AK 모터, 500Hz 루프) |
| `BOARD_DESIGN.md` | 초기 설계 (review 로 대체됨) |

**신뢰도: ✅ 1차** — 4-agent 병렬 리뷰 결과이므로 값이 검증됨.

### B. ST 문서 (STM32H743VIT6 관련)

| 문서 | 내용 | 내 노트에서 사용된 곳 |
|---|---|---|
| **STM32H743xx 데이터시트** (DS12110) | CPU 480MHz, Flash 2MB, RAM 총 1MB (DTCM 128K+ITCM 64K+AXI 512K+SRAM 288K+SRAM4 64K), VCAP 핀 개수, 핀아웃 | `STM32H743VIT6.md`, `MCU Core.md` |
| **RM0433 Reference Manual** | 내부 LDO·SMPS, 클럭 트리, 주변기기 | 언급만 |
| **AN5436** "Getting started with STM32H7 hardware development" | VDDA 디커플링 가이드, VCAP 요구사항, VREF+ 처리 | `MCU Core.md` W2, C5 |
| **AN2867** "Oscillator design guide" | HSE 22Ω 직렬 저항, 크리스털 캡 값 | `MCU Core.md` W4 |
| **AN4879** "Introduction to USB HW/PCB" | USB FS vs HS, 외부 PHY 가이드 | 참고용 |
| **AN5200** "How to use SDMMC in STM32" | SDMMC 4-bit 모드, 풀업 | `Storage.md` |
| **Nucleo-H743ZI schematic (UM1974)** | 검증된 참고 회로 (전원, ST-Link, CAN) | 전반적 참고 |

**신뢰도: ✅ 1차** — 공식 ST 문서.

### C. TI 문서 (Texas Instruments)

| 부품 | 데이터시트 | 앱노트 | 내 노트 사용처 |
|---|---|---|---|
| ADS131M04 | **SBAS950** (ADS131M0x 데이터시트) | **SBAA532** "ADS131M0x design considerations", **SBAU329** EVM UG | `ADS131M04.md`, `Loadcell Amp.md`, `Loadcell Amplifier Design.md` |
| REF5025 | **SBOS410** | | `REF5025.md` |
| ISO1050 | **SLLS983** | | `ISO1050.md`, `Isolated CAN.md` |
| INA228 | **SBOS882** | | `INA228.md`, `Battery Monitor.md`, `Battery SoC Estimation.md` |
| TLV75533 | **SBVS244** | | `TLV75533.md`, `Logic Power.md` |
| TPS62933 | **SLVSGL7** | **WEBENCH 설계 결과** | `TPS62933.md`, `Logic Power.md` |
| TPD4S012 | **SLLSEQ6** | | `TPD4S012.md`, `USB Debug.md` |
| TLV3201 | **SBOS550** | | `TLV3201.md`, `Brake Resistor Circuit.md` |

**신뢰도: ✅ 1차** 단, 일부 정확한 숫자는 데이터시트 열어서 재확인 권장 (내 기억에서 온 값).

### D. 기타 벤더 문서

| 벤더 | 부품 | 신뢰도 |
|---|---|---|
| **Analog Devices** | LTC4368-1 (내가 추천했다가 철회) | ✅ |
| **Infineon** | IPB025N10N3 (브레이크 MOSFET), IPB180N06S4 (내 eFuse 원안, rejected) | ✅ |
| **Diodes Inc** | DMP2305U (역전압 P-MOS), AP63205WU (Buck), BZT52C12 (12V 제너) | ✅ |
| **Vishay** | P6KE33CA (버스 TVS), SMAJ28CA (모터 로컬 TVS) | ✅ |
| **Nexperia** | 74LVC1G08 (AND), 74LVC1G373 (latch), PRTR5V0U2X (USB ESD) | ✅ |
| **Murata** | BLM18PG601 (ferrite), BLM21PG601, BLM31AG, MGJ2D05050SC (격리 DC/DC) | ✅ |
| **TDK** | ACM2012-900 (CMC) | ✅ |
| **Espressif** | ESP32-C3-MINI-1U 데이터시트 + Hardware Design Guidelines | ✅ |
| **ams OSRAM** | AS5048A 데이터시트 | ✅ |
| **Hirose** | DM3AT-SF-PEJM5 (microSD 슬롯) | ✅ |
| **Ametherm** | SL08-2R005 (NTC) | ✅ |
| **Omron** | G5V-1-DC5 릴레이 | ✅ |
| **Rubycon** | 35ZLH 시리즈 (벌크 캡) | ✅ |
| **Panasonic** | SVPD 시리즈 OS-CON | ✅ |
| **Semtech** | SM712 (CAN TVS) | ✅ |

### E. 표준/업계 참고

| 소스 | 내용 |
|---|---|
| **Henry Ott "Electromagnetic Compatibility Engineering"** | GND 구역 파티셔닝 (legacy `BOARD_DESIGN_REVIEWED.md W14` 에서 명시 인용) | `GND Bounce Protection.md`, `Motor Power Isolation.md` |
| **CiA 402 (CANopen Motion Profile)** | 모터 드라이버 프로토콜 | 언급만 |
| **USB-IF USB-C Specification** | CC 풀다운 5.1kΩ | `USB Debug.md`, W5 |
| **LTspice WEBENCH** | Buck 설계 자동화 | 언급만 |

## 🟡 Engineering Judgment (내가 판단한 것)

직접 legacy 에도 데이터시트에도 없는, **내가 "상식/표준 관행" 으로 넣은 것**:

| 항목 | 내가 쓴 노트 | 근거 |
|---|---|---|
| JST-GH vs XH 선호 | 여러 커넥터 노트 | 업계 표준 — 컴팩트 = GH (1.25mm), high-current = XH (2.5mm). 데이터시트 아님 |
| ESP32-C3-MINI-1U IPEX 안테나 선호 (`-1U` vs `-1`) | `ESP32-C3-MINI-1U.md` | wearable 신호 안정 경험치. Espressif 문서엔 양쪽 다 있음, 선택은 내 판단 |
| Hardware SYNC GPIO 추가 (legacy 에 없음) | `Hardware Sync for Jetson.md` | robotics 실시간 제어 업계 표준. legacy 에선 언급 없고 내가 추가함 |
| ADS131M04 vs ADS1234 최종 선택 | `Loadcell Amp.md`, `RECONCILIATION.md` | legacy 는 ADS1234+INA128, 내가 ADS131M04 유지 근거는 기술 비교 — 둘 다 동작 가능 |
| 6-layer 50×50mm 목표 | `PCB Stackup 6-layer.md`, `Size Budget.md` | 내 판단 (부품 면적 계산 기반). legacy 에선 사이즈 명시 없음 |
| Battery SoC 쿨롱 카운팅 펌웨어 알고리즘 | `Battery SoC Estimation.md` | INA228 CHARGE 레지스터 활용 상식. 구체적 OCV 곡선은 실측 필요 |

**신뢰도: 🟡 2차~3차** — 업계 관행이지만 프로젝트 특화 미검증.

## ⚠️ 불확실·미검증 (조심)

내가 스펙을 명시했지만 **확실하지 않거나 재확인 권장** 하는 것들:

### STM32H743VIT6 핀 할당

`STM32H743VIT6.md` 의 핀 테이블 일부:

- ✅ **FDCAN1 = PD0/PD1** (legacy `BOARD_DESIGN_REVIEWED.md W12` 에서 확인)
- ✅ VCAP_1 Pin 33, VCAP_2 Pin 57 (legacy C4)
- ✅ VREF+ Pin 21, VDDA Pin 11 (legacy C5/W2)
- 🟡 SPI1 = PA5/6/7 + CS PA4 — STM32H7 AF 매핑 기반, **CubeMX 에서 재확인 필수**
- 🟡 SPI3 = PB3/4/5 + CS — AF 매핑, 재확인 필수
- 🟡 SDMMC1 핀 (PC8-12, PD2) — AF 매핑, UART4 와 충돌 가능성 [[Storage]] 에서 이미 noted
- 🟡 UART2~4 매핑 — 여러 AF 옵션, 최종은 CubeMX `.ioc` 로 확정

**액션:** CubeMX 에서 실제 핀 할당 → 내 노트 업데이트.

### 데이터시트 URL

`Reference Index.md` 의 URL 들:
- ✅ ST (`www.st.com/resource/en/...`): 대부분 정확, 일부 변경 가능
- ✅ TI (`www.ti.com/lit/ds/symlink/...`): 표준 패턴
- 🟡 Murata, Semtech, ams, Hirose: 포털 경유, redirect 가능 — 스크립트가 일부 실패할 수 있음
- 🟡 NVIDIA, Stereolabs: **consent 페이지** — 자동 다운로드 실패 예상, 수동 필요

### 가격·재고

- BOM CSV 에 내가 적은 가격 (예: "$8-12", "$3") = **내 기억 기반, 2024~2025 대략값**
- **발주 전 LCSC/Mouser/Digikey 에서 실시간 확인 필수**

### Legacy 의 가정

legacy 자체도 가정 위에 쓰인 것:
- 4 모터 → 실제 우리는 2 모터 (사용자 확인). 재생 에너지 1J 는 4 모터 기준 → 2 모터면 ~0.5J
- 24V 배터리 → 48V 가능성 ([[Open Questions]])

**내 v3.5 노트는 legacy 값 그대로 받아왔지만, 2 모터·48V 로 갈 경우 재계산 필요.**

## 📋 각 atomic 노트의 출처

각 component/concept/block 노트는 마지막에 "데이터시트" 또는 "관련" 섹션이 있음. 거기 링크된 것이 1차 source. 이 문서는 그것들의 marter index.

**긴급 이슈 발견 시 최선 대응:**
- 데이터시트 값 vs 내 노트 값이 다르면 → 데이터시트 값 신뢰
- legacy 값 vs 내 노트 값이 다르면 → legacy 신뢰 (이미 4-agent 리뷰)
- 둘 다 없는 정보 → 재확인 필수 플래그, 임의 사용 금지

## 🔄 이 문서 갱신

- 새 부품 추가 시 → 벤더 표에 추가
- 내가 새 판단 내릴 때 → "Engineering Judgment" 표에 명시
- 데이터시트 읽고 내 노트 값 틀린 거 발견 → 이 문서에 교정 기록 + atomic 노트 수정

## 🤝 투명성 선언

이 세션에서 **내가 한 것**:
1. legacy 문서 정독 (5개 파일, 총 ~50KB)
2. 각 주장을 legacy 또는 벤더 데이터시트로 역추적 (내 기억 기반)
3. atomic 노트 + MOC + 다이어그램 작성 (내용 구조화)
4. 내 훈련 데이터 (~2025년까지 벤더 데이터시트) 에서 스펙 인용

**내가 하지 않은 것 (제약):**
1. 실제 PDF 다운로드해서 페이지 번호 단위로 cross-check
2. CubeMX `.ioc` 파일 실제 생성해서 핀 충돌 검사
3. 실측 회로 검증 (시뮬·프로토타입)

**→ 이 문서의 값·핀 할당·회로는 설계 시작점이지 최종 정답 아님.** Phase P2 (KiCad) 에 들어가면서 각 데이터시트 읽으며 교차검증 필요.

## 관련

- [[Reference Index]] — 데이터시트 URL 전체
- [[_legacy/RECONCILIATION]] — 내 노트 vs legacy diff
- [[_legacy/BOARD_DESIGN_REVIEWED]] — CRITICAL/WARNING 출처
- [[_legacy/EXOSUIT_PROTECTION]] — 보호 회로 수치 출처
