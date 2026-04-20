---
aliases: [References]
tags: [moc, references]
---

# Reference Index

외부 자료 인덱스. PDF는 `50 References/Datasheets/` 등에 저장.

## 블록별 참고 회로

| 블록 | 부품 | 어디서 받기 | 상태 |
|---|---|---|---|
| [[MCU Core]] | [[STM32H723VGT6]] | ST AN5436 "Hardware checklist for STM32H7" + 데이터시트 | ⬜ |
| MCU 클럭 | HSE 25MHz | ST AN2867 "Oscillator design guide" | ⬜ |
| USB HS | STM32H7 OTG_HS | Reference manual + AN4879 | ⬜ |
| [[Loadcell Amp]] | [[ADS131M04]] | TI 데이터시트 + EVM schematic | ⬜ |
| 레퍼런스 | [[REF5025]] | TI 데이터시트 | ⬜ |
| [[Encoder]] | [[AS5048A]] | ams 데이터시트 + Adafruit breakout | ⬜ |
| [[Comms]] | [[TCAN1462]] | TI 데이터시트 typical application | ⬜ |
| [[Wireless]] | [[ESP32-C3-MINI-1U]] | Espressif 데이터시트 + HW Design Guidelines | ⬜ |
| [[USB Debug]] | USB-C | USB-IF + Sparkfun USB-C breakout | ⬜ |
| ESD | [[TPD4S012]] | TI 데이터시트 | ⬜ |
| [[Storage]] | SDMMC | ST AN5200 + Nucleo schematic | ⬜ |
| [[Logic Power]] | [[TPS62933]] | TI WEBENCH + 데이터시트 | ⬜ |
| LDO | [[TLV75533]] | TI 데이터시트 | ⬜ |
| [[Battery Monitor]] | [[INA228]] | TI 데이터시트 | ⬜ |
| [[Safety UI]] | [[74LVC1G08]] | Nexperia 데이터시트 | ⬜ |
| [[Motor Power]] (B) | [[LTC4368-1]] | Analog Devices 데이터시트 + demo board | ⬜ |

## 오픈소스 보드 참고

| 보드 | 왜 보는지 | 위치 |
|---|---|---|
| ST Nucleo-H723ZG | H7 표준 회로 (전원, 디커플링, ST-Link) | ST 공식 schematic PDF |
| OpenExo (NAU) | 같은 도메인 (exo) 회로 패턴 | `../Walker_Main/Hardware/PCB/CAN_PCB/` |
| MJBots Moteus | STM32 + CAN-FD 단일보드 | github.com/mjbots/moteus |
| Elmo Solo Whistle | 모터 드라이버 후보 interface spec | Elmo Application Note |

## 작업 순서

1. PDF/zip → `Datasheets/`, `App Notes/`, `Reference Boards/` 에 저장
2. 각 블록 노트에 핵심 회로 발췌
3. KiCad 들어가기 전 핀맵 충돌 검사 (CubeMX `.ioc`)
4. Phase 3 (KiCad 프로젝트 생성)
