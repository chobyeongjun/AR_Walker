---
aliases: [References, Reference Index, Datasheets Index]
tags: [moc, references]
---

# Reference Index

> 확정된 부품 전체의 데이터시트·앱노트·레퍼런스 보드 인덱스.
> **Phase A + 확정된 Phase B 보호부품** 포함. 모터·모터 드라이버·배터리 (구체 모델) 제외.
>
> **범례:** ✅ 파일 있음 · ⏳ 브라우저 수동 다운로드 필요 · ⬜ 낮은 우선순위

## 📂 폴더 구조

```
50 References/
├── Datasheets/
│   ├── 01_MCU/                    (STM32H743, AN5437, AN2867, Nucleo)
│   ├── 02_ADC_Loadcell/           (ADS131M04, REF5025)
│   ├── 03_Encoder/                (AS5048A — v3.11 제거, 파일 보관)
│   ├── 04_CAN_Isolation/          (ISO1050, MGJ2D05050SC, ACM2012, SM712, PRTR5V0U2X)
│   ├── 05_USB_Debug/              (TPD4S012, USB-C connector)
│   ├── 06_Battery_Monitor/        (INA228, CSS2H shunt)
│   ├── 07_Logic_Power/            (TPS62933, AP63205WU, TLV75533, BLM18PG601)
│   ├── 08_Protection_Reverse/     (DMP2305U, BZT52C12)
│   ├── 09_Protection_Inrush/      (SL08-2R005, Omron G5V-1)
│   ├── 10_Protection_Bus/         (P6KE33CA, Rubycon 35ZLH)
│   ├── 11_Protection_Local/       (SMAJ28CA, SVPD471M35 OS-CON, BLM31AG)
│   ├── 12_Protection_Brake/       (TLV3201, IPB025N10N3, HS50 3R0)
│   ├── 13_Safety_UI/              (74LVC1G08, 74LVC1G373 optional)
│   ├── 14_Storage/                (Hirose DM3AT, AN5200)
│   ├── 15_Wireless/               (ESP32-C3-MINI-1U, HW Design Guide)
│   ├── 16_Passives_Grounding/     (BLM21PG601 star ferrite)
│   └── 17_System_Modules/         (EBIMU24GV6 + EBRCV24GV6, ZED X Mini, Jetson Orin NX)
├── App Notes/
└── Reference Boards/              (Nucleo-H743ZI, Moteus, ODrive)
```

## 🔗 전체 데이터시트 목록

### 01. MCU 코어 ⚠️ 전부 수동 다운로드 필요

| 부품 | 파일명 | 상태 | 수동 다운로드 URL |
|---|---|:-:|---|
| [[STM32H743VIT6]] | `STM32H743xx.pdf` | ✅ | https://www.st.com/en/microcontrollers-microprocessors/stm32h743vi.html |
| AN5437 "STM32H7 HW Checklist" | `AN5437_H7_HW_Checklist.pdf` | ⏳ | https://www.st.com/en/embedded-software/an5437-stm32h7-hardware-design-checklist.html |
| AN2867 "Oscillator Design Guide" | `AN2867_Oscillator.pdf` | ✅ | https://www.st.com/en/embedded-software/an2867-guidelines-for-oscillator-design-on-stm8afals-and-stm32-mcusmpus.html |
| AN4879 "USB hardware/PCB guide" | `AN4879_USB.pdf` | ✅ | https://www.st.com/en/embedded-software/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus.html |
| UM2407 Nucleo-H743ZI2 유저 매뉴얼 | `UM2407_Nucleo_H743ZI2.pdf` | ✅ | https://www.st.com/en/evaluation-tools/nucleo-h743zi2.html |
| Nucleo-H743ZI schematic pack | `Nucleo-H743ZI_SCH.pdf` | ✅ | https://www.st.com/en/evaluation-tools/nucleo-h743zi2.html → "Schematics" |

### 02. 로드셀 ADC 체인

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[ADS131M04]] | `ADS131M04.pdf` | ✅ |
| SBAA532 "ADS131M0x design" | `SBAA532_ADS131M_Design.pdf` | ✅ |
| ADS131M04EVM User Guide | `ADS131M04EVM_UG.pdf` | ✅ |
| [[REF5025]] | `REF5025.pdf` | ✅ |

> ⏳ TI 수동: https://www.ti.com/product/REF5025 · https://www.ti.com/product/ADS131M04

### 03. 인코더 (v3.11 제거 — 파일만 보관)

| 부품 | 파일명 | 상태 | 비고 |
|---|---|:-:|---|
| [[AS5048A]] | `AS5048A.pdf` | ✅ | v3.11 설계에서 제거됨, 파일은 참조용 보관 |

### 04. CAN 격리 체인

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[ISO1050]] | `ISO1050.pdf` | ✅ |
| [[MGJ2D05050SC]] | `MGJ2D05050SC.pdf` | ⏳ |
| ACM2012-900-2P | `ACM2012_series.pdf` | ⏳ |
| SM712-02HTG | `SM712.pdf` | ⏳ |
| PRTR5V0U2X | `PRTR5V0U2X.pdf` | ✅ |
| BZT52C5V6 | `BZT52_series.pdf` | ✅ |

> ⏳ Murata MGJ2: https://www.murata.com/en-us/products/productdetail?partno=MGJ2D05050SC
> ⏳ TDK ACM2012: https://product.tdk.com/en/search/emc/emc/cmf/info?part_no=ACM2012-900-2P-T002
> ⏳ Semtech SM712: https://www.semtech.com/products/protection/tvs-diodes/sm712-02htg

### 05. USB 디버그

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[TPD4S012]] | `TPD4S012.pdf` | ✅ |
| GCT USB4105 USB-C | `USB4105_USB-C.pdf` | ✅ |

### 06. 배터리 모니터

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[INA228]] | `INA228.pdf` | ✅ |
| Susumu CSS2H 1mΩ shunt | `CSS2H_shunt.pdf` | ⬜ |

### 07. 로직 전원

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[TPS62933]] | `TPS62933.pdf` | ✅ |
| [[AP63205WU]] | `AP63205_family.pdf` | ✅ |
| [[TLV75533]] | `TLV75533.pdf` | ✅ |
| XAL4040 inductor | `XAL4040.pdf` | ✅ |
| BLM18PG601 ferrite | `BLM18PG601.pdf` | ⬜ |

> ⏳ TI TLV75533: https://www.ti.com/product/TLV75533
> ⏳ Coilcraft XAL4040: https://www.coilcraft.com/en-us/products/power/shielded-inductors/xal/xal4040/

### 08. 역전압 보호

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[DMP2305U]] | `DMP2305U.pdf` | ✅ |
| BZT52C12 | `BZT52_series.pdf` | ✅ |

### 09. 인러시 제한

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[SL08-2R005]] NTC | `SL08_series.pdf` | ⏳ |
| Omron G5V-1-DC5 릴레이 | `Omron_G5V-1.pdf` | ✅ |

> ⏳ Ametherm SL08: https://www.ametherm.com/inrush-current-limiters/sl08-2r005

### 10. 버스 보호 (Phase B 24V 기준)

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[P6KE33CA]] | `P6KE_series.pdf` | ✅ |
| Rubycon 35ZLH4700M | `Rubycon_ZLH.pdf` | ✅ |

### 11. 모터별 로컬 보호 (Phase B)

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[SMAJ28CA]] | `SMAJ_series.pdf` | ✅ |
| Panasonic SVPD471M35 OS-CON | `OS-CON_SVPD.pdf` | ✅ |
| Murata BLM31AG601 | `BLM31AG601.pdf` | ⬜ |

### 12. 브레이크 회로

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[TLV3201]] 비교기 | `TLV3201.pdf` | ✅ |
| [[IPB025N10N3]] N-MOSFET | `IPB025N10N3G.pdf` | ✅ |
| Arcol HS50 3Ω/50W | `Arcol_HS50.pdf` | ⬜ |

### 13. 안전·UI

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[74LVC1G08]] AND gate | `74LVC1G08.pdf` | ✅ |
| 74LVC1G373 래치 (옵션) | `74LVC1G373.pdf` | ✅ |

> ⏳ Nexperia: https://assets.nexperia.com/documents/data-sheet/74LVC1G373.pdf

### 14. 저장 (microSD)

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[DM3AT microSD]] push-push | `DM3AT_microSD.pdf` | ✅ |
| AN5200 "STM32 SDMMC Guide" | `AN5200_SDMMC.pdf` | ✅ |

> ⏳ Hirose DM3AT: https://www.hirose.com/en/product/p/CL0580-2142-2-51
> ⏳ ST AN5200: https://www.st.com/en/embedded-software/an5200-sdmmc-stm32.html

### 15. 무선 (BLE/WiFi)

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[ESP32-C3-MINI-1U]] | `ESP32-C3-MINI-1U.pdf` | ✅ |
| ESP32-C3 HW Design Guidelines | `ESP32-C3_HW_Design.pdf` | ✅ |

> ⏳ Espressif: https://www.espressif.com/en/support/documents/technical-documents (ESP32-C3 Hardware Design Guidelines)

### 16. 그라운딩·패시브

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| Murata BLM21PG601 star ground | `Murata_BLM_ferrites.pdf` | ⏳ |

> ⏳ Murata: https://www.murata.com/en-us/products/productdetail?partno=BLM21PG601SN1D

### 17. 시스템 모듈 (보드 외부)

| 부품 | 파일명 | 상태 |
|---|---|:-:|
| [[EBIMU24GV6]] 센서 (V6) | `EBIMU24GV6_IMU_Module.pdf` | ✅ |
| EBRCV24GV6 수신기 (V6) | `EBRCV24GV6_Receiver.pdf` | ✅ |
| [[ZED X Mini]] 카메라 | `ZED_X_Datasheet_v1.2.pdf` | ✅ |
| [[Jetson Orin NX]] 16GB | `Jetson_Orin_NX.pdf` | ⏳ |
| Jetson Orin NX Design Guide | `Jetson_Orin_NX_DesignGuide.pdf` | ⏳ |

> ⏳ NVIDIA developer: https://developer.nvidia.com/embedded/downloads (Jetson Orin NX Datasheet & Design Guide)

---

## ⏳ 수동 다운로드 요약 (브라우저 필요)

**Phase A 필수 (먼저):**

| 파일 | 사이트 |
|---|---|
| `01_MCU/STM32H743xx.pdf` | st.com → STM32H743VI → Datasheet |
| `01_MCU/AN5437_H7_HW_Checklist.pdf` | st.com → AN5437 검색 |
| `01_MCU/Nucleo-H743ZI_SCH.pdf` | st.com → NUCLEO-H743ZI2 → Schematics |
| `01_MCU/UM2407_Nucleo_H743ZI2.pdf` | st.com → NUCLEO-H743ZI2 → User Manual |
| `07_Logic_Power/TLV75533.pdf` | ti.com → TLV75533 → Datasheet |
| `02_ADC_Loadcell/REF5025.pdf` | ti.com → REF5025 → Datasheet |

**Phase A 보조:**

| 파일 | 사이트 |
|---|---|
| `01_MCU/AN2867_Oscillator.pdf` | st.com → AN2867 |
| `01_MCU/AN4879_USB.pdf` | st.com → AN4879 |
| `14_Storage/AN5200_SDMMC.pdf` | st.com → AN5200 |
| `02_ADC_Loadcell/ADS131M04EVM_UG.pdf` | ti.com → ADS131M04EVM → User Guide |
| `13_Safety_UI/74LVC1G373.pdf` | assets.nexperia.com/documents/data-sheet/74LVC1G373.pdf |
| `14_Storage/DM3AT_microSD.pdf` | hirose.com → DM3AT-SF-PEJM5 |
| `15_Wireless/ESP32-C3_HW_Design.pdf` | espressif.com → Technical Documents |

**Phase B / 낮은 우선순위:**

| 파일 | 사이트 |
|---|---|
| `04_CAN_Isolation/MGJ2D05050SC.pdf` | murata.com → MGJ2D05050SC |
| `04_CAN_Isolation/SM712.pdf` | semtech.com → SM712-02HTG |
| `04_CAN_Isolation/ACM2012_series.pdf` | product.tdk.com → ACM2012-900-2P-T002 |
| `07_Logic_Power/XAL4040.pdf` | coilcraft.com → XAL4040 |
| `09_Protection_Inrush/SL08_series.pdf` | ametherm.com → SL08-2R005 |
| `17_System_Modules/Jetson_Orin_NX.pdf` | developer.nvidia.com → Jetson downloads |
| `17_System_Modules/Jetson_Orin_NX_DesignGuide.pdf` | developer.nvidia.com → Jetson downloads |

---

## 🔗 오픈소스 레퍼런스 보드

| 보드 | 왜 보는지 | 위치 |
|---|---|---|
| **ST Nucleo-H743ZI** ⭐ | H743 표준 회로 (전원, 디커플링, ST-Link, USB) — Phase A 펌웨어 선개발용 | `Reference Boards/Nucleo-H743ZI_SCH.pdf` (⏳ 수동) |
| MJBots Moteus r4.5 | STM32 + CAN-FD 단일보드 레퍼런스 | https://github.com/mjbots/moteus/tree/main/hw |
| ODrive v3.6 | 모터 전원 보호·CAN·USB 레퍼런스 | https://github.com/odriverobotics/ODrive/tree/master/docs |
| TI TIDA-01255 | 48V CAN 보호 레퍼런스 (48V 가면 참고) | https://www.ti.com/tool/TIDA-01255 |

## 🤖 모터 관련 (Phase B, 아직 미확정)

모터 / 모터 드라이버 / 배터리 구체 모델은 [[Open Questions]] 에서 **모터 연속 전류 계산 후 결정**. 결정되면 여기에 다음 카테고리 추가:

- `18_Motor/` — 선정된 모터 모델 데이터시트 (U8 Lite / RI60 / EC-i 40 중 1)
- `19_Motor_Driver/` — Elmo 데이터시트 + CANopen CiA 402 매뉴얼
- `20_Battery/` — 선정된 Li-ion 셀 + BMS 스펙

## 📥 다운로드 스크립트

`50 References/Datasheets/download_all.sh` — 자동 다운로드 가능한 파일 처리.
**ST·TI·Murata·Nexperia·NVIDIA 는 Cloudflare 차단으로 모두 브라우저 수동 다운로드 필요.**

## 작업 순서

1. ⏳ Phase A 필수 6개 브라우저 수동 다운로드 → `01_MCU/`, `07_Logic_Power/`, `02_ADC_Loadcell/`
2. 각 블록 노트 (예: [[MCU Core]]) 에 핵심 회로 발췌
3. KiCad 들어가기 전 핀맵 충돌 검사 (CubeMX `.ioc`)
4. Phase 3 KiCad 프로젝트 생성

## 관련

- [[Exosuit Board MOC]]
- [[BOM MOC]] — 실제 발주 리스트
- [[Schematic Blocks MOC]] — 블록별 사용 위치
