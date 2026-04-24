---
aliases: [Decisions Log, Decisions]
tags: [project, decisions]
---

# Decisions Log

시간순 기록. 근거 상세는 [[Citations & Sources]] 및 각 `[[wikilink]]` 노트 참고.

## 2026-04-17 (내 atomic 노트 세션 초기)

| 항목 | 결정 | 관련 | 상태 |
|---|---|---|---|
| 모터 모델 | 보류 (Phase 5) — Elmo 가정 | [[Open Questions]] | legacy 에선 3 후보 명시 |
| 모터 인터페이스 | CANopen | [[Comms]] | legacy 동일 |
| 로드셀 ADC | HX711 → ADS131M04 | [[Loadcell Amplifier Design]] | **v3.5 후 유지** |
| 로깅 | 온보드 microSD SDMMC 4-bit | [[Storage]] | |
| IMU | EBIMU 무선 | [[Goals & Requirements]] | legacy 에선 V5 명시 |
| EtherCAT | 사용 안 함 | | |
| 모터 전원 보드 통과 | 확정 | | |
| 모터 전류 | 15A+ 연속 / 30A 피크 | | legacy 는 계산식 명시 |
| PCB | 6-layer | [[PCB Stackup 6-layer]] | |
| 외/내층 동박 | 2oz / 1oz | | |
| 표면 처리 | ENIG | | |
| 배터리 모니터 | INA228 | [[Battery SoC Estimation]] | **v3.5 에서 × 2 로 변경** |

## 2026-04-20 오전 (Obsidian vault 재구성)

| 항목 | 결정 | 관련 |
|---|---|---|
| Vault 구조 | Obsidian atomic + MOC + wikilink | [[Exosuit Board MOC]] |
| 통합 vault | `~/research-vault/` 단일 + symlink | |
| symlink 이름 | `stm-board` | |

## 2026-04-20 오후 v3.5 (Legacy Reconciliation)

Legacy 설계 문서 발견 → 주요 결정 교정. 상세: [[_legacy/RECONCILIATION]]

| 항목 | 내 원안 | Legacy | **최종** | 변경 이유 |
|---|---|---|---|---|
| MCU | H723VGT6 | H743VIT6 | [[STM32H743VIT6]] | Flash/RAM/dual-bank/Nucleo |
| 제어 루프 | 1 kHz | 500 Hz | 500 Hz | AR_Walker 기존값 |
| CAN 트랜시버 | TCAN1462 | ISO1050 | [[ISO1050]] | GND bounce 2.5V 파손 |
| 격리 DC/DC | 없음 | MGJ2D05050SC | [[MGJ2D05050SC]] | ISO1050 VCC2 |
| 로드셀 ADC | ADS131M04 | ADS1234 + INA128UA | **ADS131M04 유지** | 기술 비교 후 사용자 "위 권고대로" |
| 배터리 모니터 | INA228 × 1 | INA228 × 2 | × 2 | 직렬 배터리 |
| 배터리 | 24~48V 미정 | 6S Li-ion 25.2V | 6S 25.2V | legacy |
| IMU | 일반 EBIMU | EBIMU V5 (2.4GHz) | EBIMU V5 | |
| 카메라 | 없음 | ZED X Mini | ZED X Mini | |
| Jetson | 일반 Orin | Orin NX 16GB | Orin NX 16GB | |
| 모터 제어 모드 | MIT 토크 | Servo + SET_POS | Servo + SET_POS | 풀리 r(θ) 가변 |

### v3.5 신규 개념

- [[Regen Energy Protection]] · [[GND Bounce Protection]] · [[Isolated CAN]] · [[Brake Resistor Circuit]] · [[Inrush Current Limiting]]

### v3.5 회로 필수 수정 (legacy C4~C7, W2~W7)

VCAP_2 (Pin 57), VREF+, VDDA 페라이트, HSE 22Ω, BOOT0, NRST, CAN STBY.

## 2026-04-20 v3.7~3.8 (투명성 강화)

| 항목 | 내용 |
|---|---|
| v3.7 | [[Citations & Sources]] 신규 — 출처 추적 문서 |
| v3.8 | [[Unsourced Items Audit]] 신규 — 내 추가 13개 항목 감사 |

## 2026-04-20 v3.9 (Unsourced Audit 답변 반영)

사용자 방침: **"너의 생각으로 하는 것들은 모두 X. 무조건 근거 있이."**

| Q | 결정 | 근거 |
|---|---|---|
| Q1 ESP32-C3-MINI-1U BLE | ✅ **포함 확정** | 사용자: "BLE 반드시 필요" |
| Q2 Hardware SYNC GPIO | ❌ **제거** | 사용자: "왜 필요한 지 모르겠다" → "모르면 제거" 원칙 |
| Q3 JST-GH 1.25mm 통일 | ✅ **확정** (근거 포함) | JST 카탈로그 — 1A+잠금+최소크기 교차점 (SH/ZH/PH/XH 대비) |
| Q4 보드 사이즈 50×50 | ⏸️ **잠정** | 사용자: "motor 정해지면 재결정" |

### 핀 할당 재평가

FDCAN1 (PD0/1), VCAP (Pin 33, 57), VREF+ (Pin 21), VDDA (Pin 11), USB FS (PA11/12), HSE (PH0/1), SWD (PA13/14) 외 모든 핀 — **내 AF 매핑 추측이라 🚩 CubeMX 확정 필요** 태그.

### ESP32 variant (-1U vs -1)

IPEX vs PCB 내장 — legacy 에 없고 내 판단 → **사용자 확정 보류**. 현재 `-1U` 가정 유지하되, 케이스 설계 때 재확인.

### BLE 용도 명확화

**BLE 는 500Hz 저수준 제어 불가** (Bluetooth Core 5.0 — connection interval 최소 7.5ms). BLE 역할:
- 고수준 명령 (1~10 Hz)
- 텔레메트리 (10~100 Hz)
- OTA
- 파라미터 튜닝

500Hz 제어는 STM32 내부.

## 2026-04-20 v3.10 (SYNC 재추가)

| 항목 | 결정 | 근거 |
|---|---|---|
| Hardware SYNC GPIO | ✅ **재추가** (v3.9 제거 철회) | 사용자: "SYNC 추가야" |

v3.9 에서 "잘 모르겠다" → 제거했으나 v3.10 에서 명시 추가 결정. 핀 PB14 (SYNC_OUT) / PB15 (SYNC_IN) 🚩 CubeMX 확정 필요. JST-GH 3pin 커넥터 복원. [[Hardware Sync for Jetson]] status 를 `removed` → `decided` 로 전환.

## 2026-04-22 (IMU V5 → V6 정정)

| 항목 | 정정 내용 |
|---|---|
| IMU 모델 | legacy HANDOFF "EBIMU V5" → 실제 사용 **V6**. 모델명: IMU `EBIMU24GV6` + 수신기 `EBRCV24GV6`. 사용자 데이터시트 (`EBIMU24GV6_rev10.pdf`, `EBRCV24GV6_rev10.pdf`) 확인. |
| 노트 rename | `EBIMU EBMotion V5.md` → `EBIMU24GV6.md`. 모든 wikilink `[[EBIMU EBMotion V5]]` → `[[EBIMU24GV6]]` 또는 alias 자동 해결. |

## 2026-04-23 v3.11 (AS5048A 제거)

| 항목 | 결정 | 근거 |
|---|---|---|
| AS5048A 풀리 인코더 ×2 | ❌ **제거** | 모터 포지션(Elmo CANopen)으로 케이블 길이 계산 가능 + ZED 카메라로 지체 위치 확인 → 고유 정보 없음. 유일한 고유값(풀리 슬립 감지)은 이 시스템의 현실적 고장 모드가 아님. SPI3 버스 전체 제거. |

## 2026-04-24 P1 완료 (레퍼런스 수집)

| 항목 | 결정 | 근거 |
|---|---|---|
| HW 설계 가이드 | AN5437 아닌 **AN4938** | "Getting started with STM32H74x hardware development" — H743이 H74x 패밀리. AN5437은 자동 다운로드 실패 + 실제 적용 문서는 AN4938 |
| 레퍼런스 보드 | MJBots Moteus r4.5 + ODrive v3.5 추가 | GitHub에서 Eagle sch/brd 파일 수집 (STM32 + CAN 설계 참조용) |
| Modular Strategy 정정 | H723→H743, TCAN1462→ISO1050+MGJ2D05050SC, AS5048A 행 제거 | v3.11 반영 누락 수정 |
| P1 상태 | ✅ **완료** | Phase A 필수 데이터시트 전부 수집 완료 |

## 템플릿

공용 템플릿 (Component / Concept / Decision) 은 `~/research-vault/templates/` 에, 프로젝트 전용 ([[Schematic Block]]) 은 `90 Templates/`.
