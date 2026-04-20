---
aliases: [Citations, Sources, Provenance]
tags: [project, references]
---

# Citations & Sources

> 이 vault 의 각 주장·스펙이 어디서 왔는지 투명하게 기록.
> 사용자 방침 (2026-04-20): **"너의 생각으로 하는 것들은 모두 X. 무조건 근거 있이."**

## 📌 분류

1. **벤더 데이터시트** = 1차 출처
2. **Legacy 문서** (4-agent 리뷰 거침) = 검증된 2차
3. **사용자 세션 확정** = 합의 사항
4. **불확실 / 내 추측** = **🚩 제거 또는 확정 필요**

## 📚 Primary Sources

### A. Legacy 문서 (2026-04-12~14)

`10 Project/_legacy/` 에 원본 보존.

| 파일 | 제공하는 스펙 |
|---|---|
| `HANDOFF.md` | MCU=H743, IMU=EBIMU V5, 카메라=ZED X Mini, Jetson=Orin NX 16GB, 제어=500Hz 어드미턴스, 모터 모드=Servo+SET_POS |
| `BOARD_DESIGN_REVIEWED.md` | CRITICAL C1~C7, WARNING W1~W15 (VCAP_2 Pin 57, VREF+ Pin 21, FDCAN1 PD0/PD1, HSE 22Ω, Buck AP63205WU 등) |
| `EXOSUIT_PROTECTION.md` | 재생 에너지 1J·143V·11,280µF, GND bounce 2.5V·50A/µs, 브레이크 임계 26.5V, 보호 부품 전체 |

### B. ST 문서

| 문서 | 제공 정보 |
|---|---|
| STM32H743xx 데이터시트 (DS12110) | CPU 480MHz, Flash 2MB, RAM 1MB, 핀아웃, AF 매핑 |
| RM0433 Reference Manual | 주변기기 상세 |
| AN5436 Hardware checklist | VDDA 디커플링, VCAP, VREF+ |
| AN2867 Oscillator design | HSE 22Ω, 캡 값 |
| AN5200 SDMMC Guide | 풀업, 4-bit 모드 |
| Nucleo-H743ZI UM1974 | 검증된 참고 회로 |

### C. TI 문서

| 부품 | 데이터시트 | 앱노트 |
|---|---|---|
| ADS131M04 | SBAS950 | SBAA532, SBAU329 EVM UG |
| REF5025 | SBOS410 | |
| ISO1050 | SLLS983 | |
| INA228 | SBOS882 | |
| TLV75533 | SBVS244 | |
| TPS62933 | SLVSGL7 | WEBENCH |
| TPD4S012 | SLLSEQ6 | |
| TLV3201 | SBOS550 | |

### D. 기타 벤더

| 벤더 | 부품 |
|---|---|
| Infineon | IPB025N10N3 |
| Diodes Inc | DMP2305U, AP63205WU, BZT52 |
| Vishay | P6KE33CA, SMAJ28CA |
| Nexperia | 74LVC1G08, PRTR5V0U2X |
| Murata | BLM18PG601, BLM21PG601, BLM31AG, MGJ2D05050SC |
| TDK | ACM2012-900 |
| Espressif | ESP32-C3-MINI-1U + HW Design Guidelines |
| ams OSRAM | AS5048A |
| Hirose | DM3AT-SF-PEJM5 |
| Ametherm | SL08-2R005 |
| Omron | G5V-1-DC5 |
| Rubycon | 35ZLH |
| Panasonic | SVPD OS-CON |
| Semtech | SM712 |

### E. 표준 / 업계

| 소스 | 내용 | 적용 |
|---|---|---|
| **Henry Ott "EMC Engineering"** | GND 구역 파티셔닝 (legacy W14 명시 인용) | [[GND Bounce Protection]], [[Motor Power Isolation]] |
| **CiA 402 (CANopen Motion Profile)** | 모터 드라이버 프로토콜 | 언급 |
| **USB-IF USB-C Spec** | CC 풀다운 5.1kΩ | legacy W5 |
| **JST 공식 카탈로그** (jst-mfg.com) | GH 1.25mm: 1A + locking + 신호용. SH/ZH/PH/XH 비교 | [[Citations & Sources#F. 커넥터 근거]] |
| **Bluetooth Core Spec 5.0** | BLE connection interval 7.5ms 최소 | [[ESP32-C3-MINI-1U]] |
| **IPC-2221** | 트레이스 너비 vs 전류 | 모터 폴리곤 설계 |

### F. 커넥터 근거 (JST-GH 1.25mm 통일)

**사용자 확정 + JST 카탈로그 근거.**

JST 커넥터 시리즈 공식 비교 (jst-mfg.com):

| 시리즈 | Pitch | 전류/pin | 잠금 | 선정 여부 |
|---|---:|---:|:-:|---|
| SH | 1.0 mm | 1 A | ❌ | 잠금 없음 → wearable 탈락 |
| **GH** ⭐ | **1.25 mm** | **1 A** | **✅** | **선정: 우리 요구 완벽** |
| ZH | 1.5 mm | 1 A | ❌ | 잠금 없음 탈락 |
| PH | 2.0 mm | 2 A | ❌ | 잠금 없음 탈락 |
| XH | 2.5 mm | 3 A | ✅ | 크기 2× → 컴팩트 탈락 |

우리 요구 교차점:
- 신호 전류 < 1A (모든 센서·통신) ✓
- 잠금 (wearable 진동) ✓
- 최소 크기 (컴팩트) ✓

→ **GH 가 유일한 해.**

고전류 (모터 전원 15A+) 는 별도 — XT30/XT60 사용 (Phase B).

## ✅ 사용자 확정 사항 (2026-04-20 세션)

| 항목 | 결정 | 근거 |
|---|---|---|
| MCU H743VIT6 | legacy + 내가 비교 → 사용자 "위 권고대로" | ✅ |
| 로드셀 ADS131M04 (legacy ADS1234 에서 변경) | 사용자 "위 권고대로" | ✅ |
| CAN ISO1050 격리 | 사용자 "OK" (GND bounce 설명 후) | ✅ |
| 6-layer PCB | 사용자 "Layer 높여서 사이즈 줄이자" | ✅ |
| ESP32-C3-MINI-1U BLE 포함 | 사용자 "BLE 반드시 필요" | ✅ |
| JST-GH 1.25mm 통일 | 사용자 "통일하는 건 좋음" + JST 카탈로그 근거 | ✅ |
| E-stop 포함 | 사용자 "긴급정지도 만들어야 해" | ✅ |
| Hardware SYNC GPIO 포함 (v3.10) | 사용자: "SYNC 추가야" (v3.9 제거 철회) | ✅ |
| 보호 아키텍처 (regen/brake/inrush/reverse/GND bounce) | 사용자 "legacy 그대로" + _legacy 출처 | ✅ |

## ⏸️ 잠정 (사용자 방침: "나중에 결정")

| 항목 | 상태 | 확정 시점 |
|---|---|---|
| 보드 사이즈 50×50mm | 잠정 | P6 레이아웃 / 모터 확정 후 |
| ESP32 antenna variant (-1 vs -1U) | 잠정 | 케이스 설계 시 |
| 모터 모델 (3 후보) | 보류 | 전류 계산 후 |
| 모터 드라이버 (Elmo 3 후보) | 보류 | 모터 후 |
| 배터리 24V vs 48V | 보류 | 모터 속도 후 |

## 🔄 결정 번복 이력

| 항목 | 원래 | v3.9 | v3.10 |
|---|---|---|---|
| Hardware SYNC GPIO | 내 추가 | ❌ 제거 (사용자 "모르겠다") | ✅ 재추가 (사용자 "SYNC 추가야") |

→ 최종: 포함. [[Hardware Sync for Jetson]]

## 🚩 불확실 (재확인 필요)

### STM32H743VIT6 핀 할당

✅ **Legacy 또는 ST 고정핀 확정:**
- HSE PH0/PH1, LSE PC14/PC15, SWDIO PA13, SWCLK PA14
- VCAP_1 Pin 33, VCAP_2 Pin 57, VDDA Pin 11, VREF+ Pin 21
- FDCAN1 PD0/PD1, USB FS PA11/PA12

🚩 **CubeMX `.ioc` 에서 확정 필요 (내 추측):**
- SPI1 (PA5/6/7 + PA4 CS)
- SPI3 (PB3/4/5) — SWD SWO 와 PB3 공유 주의
- I²C1 (PB8/PB9)
- UART2/3/4 — 특히 UART4 ↔ SDMMC1 D2/D3 충돌 가능
- TIM1 PWM, ESTOP (PC13), MOTOR_ENABLE

### 데이터시트 URL

일부 redirect 또는 consent 페이지 있음 (Hirose, Stereolabs, EBIMU) — `download_all.sh` 실행 후 Failed 로그 수동 보충.

### 가격 정보

BOM 의 가격 (~$8-12 등) 은 **내 기억 기반 2024~2025 대략**. 발주 전 LCSC/Mouser/Digikey 실시간 확인 필수.

### Legacy 의 가정 → 우리 시스템과 다른 부분

- Legacy: **4 모터** 기준. 실제 우리: **2 모터** → 재생 에너지 1J 는 4 모터 기준이므로 2 모터면 ~0.5 J (더 여유).
- Legacy: 24V 기준. 실제: 24V / 48V 미정.

벌크 캡·TVS 등 수치를 **2 모터 + 전압 확정 후 재계산** 필요.

## 🤝 투명성 선언 (중요)

**내가 한 것:**
1. Legacy 문서 5개 정독
2. 각 주장을 legacy 또는 벤더 데이터시트로 역추적 (내 훈련 기억 기반)
3. atomic 노트 + MOC + 다이어그램 구조화

**내가 하지 않은 것 (제약):**
1. 실제 PDF 다운로드해서 페이지 단위 cross-check
2. CubeMX `.ioc` 파일 실제 생성해서 핀 충돌 검사
3. 실측 회로 검증 (시뮬·프로토타입)
4. 최신 가격·재고 확인

**→ 이 vault 는 설계 시작점. 최종 정답 아님.** KiCad·CubeMX 에 들어가며 교차검증 필요.

## 관련

- [[Reference Index]] — 데이터시트 URL 전체
- [[_legacy/RECONCILIATION]] — 내 노트 vs legacy diff
- [[Unsourced Items Audit]] — 감사 결과 (v3.8~v3.9)
