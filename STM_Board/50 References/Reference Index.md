---
aliases: [References, Reference Index, Datasheets Index]
tags: [moc, references]
---

# Reference Index

> 확정된 부품 전체의 데이터시트·앱노트·레퍼런스 보드 인덱스.
> **Phase A + 확정된 Phase B 보호부품** 포함. 모터·모터 드라이버·배터리 (구체 모델) 제외.

## 📂 폴더 구조

```
50 References/
├── Datasheets/
│   ├── 01_MCU/                    (STM32H743, AN5436, AN2867, Nucleo)
│   ├── 02_ADC_Loadcell/           (ADS131M04, REF5025)
│   ├── 03_Encoder/                (AS5048A)
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

### 01. MCU 코어

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[STM32H743VIT6]] | ST | https://www.st.com/resource/en/datasheet/stm32h743vi.pdf | `STM32H743xx.pdf` | ⬜ |
| AN5436 "STM32H7 Hardware checklist" | ST | https://www.st.com/resource/en/application_note/an5437-hardware-design-for-stm32h7-series-stmicroelectronics.pdf | `AN5436_H7_HW_Checklist.pdf` | ⬜ |
| AN2867 "Oscillator Design Guide" | ST | https://www.st.com/resource/en/application_note/an2867-guidelines-for-oscillator-design-on-stm8afals-and-stm32-mcusmpus-stmicroelectronics.pdf | `AN2867_Oscillator.pdf` | ⬜ |
| AN4879 "USB HS PHY design" (참고) | ST | https://www.st.com/resource/en/application_note/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf | `AN4879_USB_HS.pdf` | ⬜ |
| Nucleo-H743ZI 유저 매뉴얼 | ST | https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf | `UM1974_Nucleo_H743ZI.pdf` | ⬜ |
| Nucleo-H743ZI schematic | ST | https://www.st.com/resource/en/schematic_pack/mb1364-h743zi-c01_schematic.pdf | `Nucleo-H743ZI_SCH.pdf` | ⬜ |

### 02. 로드셀 ADC 체인

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[ADS131M04]] | TI | https://www.ti.com/lit/ds/symlink/ads131m04.pdf | `ADS131M04.pdf` | ⬜ |
| ADS131M04EVM User Guide | TI | https://www.ti.com/lit/ug/sbau329/sbau329.pdf | `ADS131M04EVM_UG.pdf` | ⬜ |
| [[REF5025]] | TI | https://www.ti.com/lit/ds/symlink/ref5025.pdf | `REF5025.pdf` | ⬜ |
| SBAA532 "ADS131M0x design" (App Note) | TI | https://www.ti.com/lit/an/sbaa532/sbaa532.pdf | `SBAA532_ADS131M_Design.pdf` | ⬜ |

### 03. 인코더

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[AS5048A]] | ams OSRAM | https://ams-osram.com/zh-cn/products/sensor-solutions/rotary-position-sensors/ams-as5048a-rotary-position-sensor | `AS5048A.pdf` | ⬜ |
| AS5048 Adafruit breakout schematic | Adafruit | https://learn.adafruit.com/downloads/pdf/adafruit-as5048-breakout.pdf | `AS5048_Adafruit_BOB.pdf` | ⬜ |

### 04. CAN 격리 체인

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[ISO1050]] | TI | https://www.ti.com/lit/ds/symlink/iso1050.pdf | `ISO1050.pdf` | ⬜ |
| [[MGJ2D05050SC]] | Murata Power | https://power.murata.com/datasheet?/data/power/ncl/kdc_mgj2.pdf | `MGJ2D05050SC.pdf` | ⬜ |
| ACM2012-900-2P-T002 | TDK | https://product.tdk.com/en/search/inductor/emc/emifil/info?part_no=ACM2012-900-2P-T002 | `ACM2012-900.pdf` | ⬜ |
| SM712-02HTG | Semtech | https://www.semtech.com/products/protection/tvs-diodes/sm712 | `SM712.pdf` | ⬜ |
| PRTR5V0U2X | Nexperia | https://assets.nexperia.com/documents/data-sheet/PRTR5V0U2X.pdf | `PRTR5V0U2X.pdf` | ⬜ |
| BZT52C5V6 (VCC2 clamp) | Commodity | https://www.diodes.com/assets/Datasheets/ds18001.pdf | `BZT52_series.pdf` | ⬜ |

### 05. USB 디버그

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[TPD4S012]] | TI | https://www.ti.com/lit/ds/symlink/tpd4s012.pdf | `TPD4S012.pdf` | ⬜ |
| USB-C 16pin 리셉터클 (GCT USB4105) | GCT | https://gct.co/files/drawings/usb4105.pdf | `USB4105.pdf` | ⬜ |
| USB-C 스펙 (참고) | USB-IF | https://www.usb.org/documents | `USB_Type-C_Spec.pdf` | ⬜ |

### 06. 배터리 모니터

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[INA228]] | TI | https://www.ti.com/lit/ds/symlink/ina228.pdf | `INA228.pdf` | ⬜ |
| Susumu CSS2H 1mΩ shunt | Susumu | https://www.susumu.co.jp/common/pdf/n_catalog_partition09_en.pdf | `CSS2H_shunt.pdf` | ⬜ |

### 07. 로직 전원

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[TPS62933]] (48V 옵션) | TI | https://www.ti.com/lit/ds/symlink/tps62933.pdf | `TPS62933.pdf` | ⬜ |
| [[AP63205WU]] (24V 옵션) | Diodes | https://www.diodes.com/assets/Datasheets/AP63200-AP63201-AP63203-AP63205.pdf | `AP63205WU.pdf` | ⬜ |
| [[TLV75533]] | TI | https://www.ti.com/lit/ds/symlink/tlv75533.pdf | `TLV75533.pdf` | ⬜ |
| BLM18PG601SN1D (ferrite) | Murata | https://www.murata.com/products/productdata/8796738650142/ENFA0003.pdf | `BLM18PG601.pdf` | ⬜ |
| XAL4040-222 inductor | Coilcraft | https://www.coilcraft.com/getmedia/fb72e61d-5aee-47bb-b1ba-5f76a23b1d5d/xal4040.pdf | `XAL4040.pdf` | ⬜ |

### 08. 역전압 보호

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[DMP2305U]] | Diodes | https://www.diodes.com/assets/Datasheets/DMP2305U.pdf | `DMP2305U.pdf` | ⬜ |
| [[BZT52C12]] | Commodity (Diodes) | https://www.diodes.com/assets/Datasheets/ds18001.pdf | `BZT52_series.pdf` | ⬜ |

### 09. 인러시 제한

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[SL08-2R005]] NTC | Ametherm | https://www.ametherm.com/hubfs/docs/data-sheets/sl08-datasheet.pdf | `SL08_NTC.pdf` | ⬜ |
| Omron G5V-1-DC5 릴레이 | Omron | https://omronfs.omron.com/en_US/ecb/products/pdf/en-g5v_1.pdf | `Omron_G5V-1.pdf` | ⬜ |

### 10. 버스 보호 (Phase B 24V 기준)

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[P6KE33CA]] | Vishay | https://www.vishay.com/docs/88471/p6kea.pdf | `P6KE33CA.pdf` | ⬜ |
| Rubycon 35ZLH4700M 전해 | Rubycon | https://www.rubycon.co.jp/wp-content/uploads/catalog/e_pdfs/aluminum/e_ZLH.pdf | `Rubycon_ZLH.pdf` | ⬜ |

### 11. 모터별 로컬 보호 (Phase B 24V 기준)

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[SMAJ28CA]] | Vishay | https://www.vishay.com/docs/88428/smaj.pdf | `SMAJ_series.pdf` | ⬜ |
| Panasonic SVPD471M35 OS-CON | Panasonic | https://industrial.panasonic.com/cdbs/www-data/pdf/RDA0000/ABA0000C1231.pdf | `OS-CON_SVPD.pdf` | ⬜ |
| Murata BLM31AG601SN1L | Murata | https://www.murata.com/products/productdata/8796738650142/ENFA0003.pdf | `BLM31AG601.pdf` | ⬜ |

### 12. 브레이크 회로

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[TLV3201]] 비교기 | TI | https://www.ti.com/lit/ds/symlink/tlv3201.pdf | `TLV3201.pdf` | ⬜ |
| [[IPB025N10N3]] N-MOSFET | Infineon | https://www.infineon.com/dgdl/Infineon-IPB025N10N3G-DataSheet-v02_01-EN.pdf | `IPB025N10N3.pdf` | ⬜ |
| Arcol HS50 3Ω/50W 브레이크 저항 | Arcol | https://uk.farnell.com/arcol/hs50-3r0-j/resistor-3r-5-50w-wirewound/dp/1086192 | `Arcol_HS50.pdf` | ⬜ |

### 13. 안전·UI

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[74LVC1G08]] AND gate | Nexperia | https://assets.nexperia.com/documents/data-sheet/74LVC1G08.pdf | `74LVC1G08.pdf` | ⬜ |
| 74LVC1G373 래치 (옵션) | Nexperia | https://assets.nexperia.com/documents/data-sheet/74LVC1G373.pdf | `74LVC1G373.pdf` | ⬜ |

### 14. 저장 (microSD)

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[DM3AT microSD]] push-push | Hirose | https://www.hirose.com/en/product/document?clcode=CL0580-2142-2-51&productname=DM3AT-SF-PEJM5&series=DM3&documenttype=Catalog&lang=en&documentid=D31543_en | `DM3AT_microSD.pdf` | ⬜ |
| AN5200 "STM32 SDMMC Guide" | ST | https://www.st.com/resource/en/application_note/an5200-how-to-use-sdmmc-in-stm32-microcontrollers-stmicroelectronics.pdf | `AN5200_SDMMC.pdf` | ⬜ |

### 15. 무선 (BLE/WiFi)

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[ESP32-C3-MINI-1U]] | Espressif | https://www.espressif.com/sites/default/files/documentation/esp32-c3-mini-1_datasheet_en.pdf | `ESP32-C3-MINI-1U.pdf` | ⬜ |
| ESP32-C3 Hardware Design Guidelines | Espressif | https://www.espressif.com/sites/default/files/documentation/esp32-c3_hardware_design_guidelines_en.pdf | `ESP32-C3_HW_Design.pdf` | ⬜ |
| ESP32-C3 Bring-up | Espressif | https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/index.html | `ESP32-C3_bringup.pdf` | ⬜ |

### 16. 그라운딩·패시브

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| BLM21PG601SN1D star ground ferrite | Murata | https://www.murata.com/products/productdata/8796738650142/ENFA0003.pdf | `BLM21PG601.pdf` | ⬜ |

### 17. 시스템 모듈 (보드 외부)

| 부품 | 제조사 | 데이터시트 URL | 파일명 | ⬜ |
|---|---|---|---|:-:|
| [[EBIMU24GV6]] 센서 모듈 (V6) | E2BOX | 사용자 로컬 (`EBIMU24GV6_rev10.pdf`) | `EBIMU24GV6_IMU_Module.pdf` | ⬜ |
| EBRCV24GV6 수신기 (V6) | E2BOX | 사용자 로컬 (`EBRCV24GV6_rev10.pdf`) | `EBRCV24GV6_Receiver.pdf` | ⬜ |
| [[ZED X Mini]] 스테레오 카메라 | Stereolabs | https://store.stereolabs.com/products/zed-x-mini-datasheet | `ZED_X_Mini.pdf` | ⬜ |
| [[Jetson Orin NX]] 16GB | NVIDIA | https://developer.nvidia.com/downloads/embedded/l4t/r35_release_v1.0/datasheets/JETSON_ORIN_NX_MODULE_DS_10929_002.pdf | `Jetson_Orin_NX.pdf` | ⬜ |

## 🔗 오픈소스 레퍼런스 보드

| 보드 | 왜 보는지 | 위치 |
|---|---|---|
| **ST Nucleo-H743ZI** ⭐ | H743 표준 회로 (전원, 디커플링, ST-Link, RMII, USB) — **Phase A 펌웨어 선 개발용 실물 구매 권장** | `Reference Boards/Nucleo-H743ZI_SCH.pdf` |
| OpenExo (NAU) | 같은 도메인 (exo) 기존 회로 | `../Walker_Main/Hardware/PCB/CAN_PCB/` |
| MJBots Moteus r4.5 | STM32 + CAN-FD 단일보드 레퍼런스 | https://github.com/mjbots/moteus/tree/main/hw |
| ODrive v3.6 | 모터 전원 보호·CAN·USB 레퍼런스 | https://github.com/odriverobotics/ODrive/tree/master/docs |
| TI TIDA-01255 | 48V CAN 보호 레퍼런스 (48V 가면 참고) | https://www.ti.com/tool/TIDA-01255 |

## 🤖 모터 관련 (Phase B, 아직 미확정)

모터 / 모터 드라이버 / 배터리 구체 모델은 [[Open Questions]] 에서 **모터 연속 전류 계산 후 결정**. 결정되면 여기에 다음 카테고리 추가:

- `18_Motor/` — 선정된 모터 모델 데이터시트 (U8 Lite / RI60 / EC-i 40 중 1)
- `19_Motor_Driver/` — Elmo 데이터시트 + CANopen CiA 402 매뉴얼
- `20_Battery/` — 선정된 Li-ion 셀 + BMS 스펙

## 📥 다운로드 스크립트

`50 References/Datasheets/download_all.sh` 에 전 부품 다운로드 스크립트 있음. 맥 로컬에서:

```bash
cd ~/stm_board/50\ References/Datasheets
bash download_all.sh
```

일부 URL 은 2단계 redirect 또는 사용자 동의 페이지 필요 — 스크립트 돌리고 실패 로그 보면 수동 클릭 필요 파일 나옴.

## 작업 순서

1. 위 표 전체 ⬜ → ✅ 채우기 (PDF 저장)
2. 각 블록 노트 (예: [[MCU Core]]) 에 핵심 회로 발췌 + 참고 페이지 링크
3. KiCad 들어가기 전 핀맵 충돌 검사 (CubeMX `.ioc`)
4. Phase 3 KiCad 프로젝트 생성

## 관련

- [[Exosuit Board MOC]]
- [[BOM MOC]] — 실제 발주 리스트
- [[Schematic Blocks MOC]] — 블록별 사용 위치
