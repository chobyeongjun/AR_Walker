---
aliases: [Datasheets]
tags: [references]
---

# Datasheets

> 확정 부품 데이터시트 로컬 저장소. 모터·모터 드라이버·배터리 구체 모델은 Phase B 결정 후 추가.

## 📥 자동 다운로드

```bash
cd ~/stm_board/50\ References/Datasheets
bash download_all.sh
```

스크립트가 아래 17개 카테고리로 분류해서 PDF 를 받음. 일부 벤더 (Stereolabs, NVIDIA, Hirose 등) 는 consent 페이지 때문에 실패할 수 있음 — 실패 로그 보고 브라우저로 수동 다운로드.

## 📂 카테고리

| # | 폴더 | 부품 |
|---|---|---|
| 01 | `01_MCU/` | STM32H743xx, AN5436, AN2867, Nucleo-H743ZI |
| 02 | `02_ADC_Loadcell/` | ADS131M04, REF5025, EVM, SBAA532 |
| 03 | `03_Encoder/` | AS5048A |
| 04 | `04_CAN_Isolation/` | ISO1050, MGJ2D05050SC, PRTR5V0U2X, SM712, BZT52C5V6, ACM2012-900 |
| 05 | `05_USB_Debug/` | TPD4S012, USB-C GCT USB4105 |
| 06 | `06_Battery_Monitor/` | INA228 (× 2 동일 부품) |
| 07 | `07_Logic_Power/` | TPS62933, AP63205WU, TLV75533, BLM18PG601, XAL4040 |
| 08 | `08_Protection_Reverse/` | DMP2305U, BZT52C12 |
| 09 | `09_Protection_Inrush/` | SL08-2R005 NTC, Omron G5V-1 relay |
| 10 | `10_Protection_Bus/` | P6KE33CA, Rubycon 35ZLH |
| 11 | `11_Protection_Local/` | SMAJ28CA, SVPD471M35 OS-CON, BLM31AG |
| 12 | `12_Protection_Brake/` | TLV3201, IPB025N10N3, Arcol HS50 |
| 13 | `13_Safety_UI/` | 74LVC1G08, 74LVC1G373 (옵션) |
| 14 | `14_Storage/` | Hirose DM3AT, AN5200 SDMMC |
| 15 | `15_Wireless/` | ESP32-C3-MINI-1U, HW Design Guidelines |
| 16 | `16_Passives_Grounding/` | BLM21PG601 star ferrite |
| 17 | `17_System_Modules/` | EBIMU24GV6 + EBRCV24GV6, ZED X Mini, Jetson Orin NX |

## ⏳ 미확정 (Phase B)

아래 카테고리는 모터 결정 후 추가:

- `18_Motor/` — T-Motor U8 Lite / CubeMars RI60 / Maxon EC-i 40 중 1
- `19_Motor_Driver/` — Elmo Gold Twitter / Solo Twitter / Whistle 중 1 + CANopen CiA 402 매뉴얼
- `20_Battery/` — 구체 Li-ion 셀 + 배터리 팩 BMS 스펙

## 관련

- [[Reference Index]] — 전체 인덱스 (URL 포함)
- [[BOM_v3]] — 발주 리스트
