# Reference Schematics Index

각 보드 블록의 회로도 레퍼런스 위치. **schematic 그리기 전에 이 표 전체를 다 받아놔야 함.**
(형이 말한 "회로도 미리 다 찾아서 연결" 단계)

| 블록 | 부품 / 보드 | 어디서 받기 | 봐야 할 부분 | 다운로드 상태 |
|---|---|---|---|---|
| MCU 코어 | STM32H723ZG | ST AN5436 "Hardware checklist for STM32H7" + 데이터시트 | VCAP/디커플링/HSE/SWD 표준 회로 | ⬜ |
| MCU 클럭 | HSE 25MHz | ST AN2867 "Oscillator design guide" | 캡/저항 값 계산 | ⬜ |
| USB HS PHY | STM32H7 OTG_HS | STM32H7 reference manual + ST AN4879 | D+/D- 라우팅, VBUS sensing | ⬜ |
| Ethernet (TBD) | LAN8742A + RMII | Microchip LAN8742A datasheet typical app + ST Nucleo H743 schematic | 25MHz/50MHz 클럭, magnetics — EtherCAT 아닌 일반 TCP/UDP 용도 | ⬜ |
| CAN-FD | TCAN1462 | TI TCAN1462 datasheet "Typical application" | 종단/ESD/공통모드 초크 | ⬜ |
| 로드셀 ADC | ADS131M04 | TI ADS131M04 datasheet + EVM schematic | 차동 입력, AVDD/DVDD, REF, anti-alias | ⬜ |
| 전압 레퍼런스 | REF5025 | TI REF5025 datasheet | bypass, 노이즈 필터 | ⬜ |
| 엔코더 | AS5048A | ams AS5048A datasheet + Adafruit 보드 | SPI 풀업, 자석 위치 | ⬜ |
| 전류 센스 | INA240A1 | TI INA240 datasheet | 션트 RC 필터 | ⬜ |
| Buck converter | TPS54360 | TI WEBENCH 출력 schematic + 데이터시트 typical app | 인덕터/캡/피드백 분압 | ⬜ |
| LDO | TLV75533 | TI 데이터시트 typical app | 입출력 캡 | ⬜ |
| microSD | SDMMC | ST AN5200 + Nucleo schematic | 풀업, ESD, 카드 detect | ⬜ |
| USB-C 커넥터 | USB 2.0 only | USB-IF spec + Sparkfun USB-C breakout schematic | CC 5.1k 풀다운 (sink), 배향 무관 핀 묶기 | ⬜ |
| ESD | TPD4S012 / USBLC6 | TI/ST 데이터시트 | USB D+/D- TVS | ⬜ |
| Reverse polarity | P-MOSFET ideal diode | TI app note SLVA936 | 게이트/풀다운 | ⬜ |
| BLE/WiFi 모듈 | ESP32-C3-MINI-1 | Espressif "ESP32-C3-MINI-1 datasheet" + "Hardware Design Guidelines" | 안테나 keep-out, 전원 디커플링, UART, EN/IO9 부트 | ⬜ |
| ESP32 부트 회로 | ESP32-C3 | Espressif AN "ESP32-C3 Bring-up" | EN/IO9 풀업/풀다운, BOOT 버튼 | ⬜ |

## 참고 보드 schematic (오픈소스)

| 보드 | 왜 보는지 | 링크 / 위치 |
|---|---|---|
| **ST Nucleo-H723ZG** | H7 표준 회로 (전원, 디커플링, ST-Link, RMII PHY) | ST 공식 schematic PDF |
| **OpenExo (NAU)** | 같은 도메인 (exo) 회로 패턴 | `Walker_Main/Hardware/PCB/CAN_PCB/NAU_OpenExo_PCB_Files.zip` |
| **MJBots Moteus** | STM32 + CAN-FD 단일보드, 모터제어 | github.com/mjbots/moteus (open hardware) |
| **ODrive v3.6** | 모터 전원 보호회로, CAN, USB | github.com/odriverobotics/ODrive |
| **TinyMovr** | STM32G4 CAN-FD 서보보드 (참고용) | github.com/tinymovr |
| **Elmo Solo Whistle** | 우리 모터 드라이버 후보 — interface spec | Elmo "Application Note" + CANopen DS402 매뉴얼 |

## 작업 순서

1. **이 문서의 PDF/zip을 모두 `refs/datasheets/` 와 `refs/boards/` 에 저장** (링크 깨짐 방지)
2. 각 블록별로 reference circuit을 `schematic_blocks/<block>.md` 에 발췌·메모
3. KiCad 들어가기 전에 핀맵 충돌 검사 (CubeMX `.ioc`)
4. 그 다음 Phase 3 (KiCad 프로젝트 생성)

## 다음에 형/사용자에게 확인할 것

- ST Nucleo-H723ZG 보드를 갖고 있는지 (있으면 schematic 그대로 베끼면 됨)
- Elmo 후보 모델 (Solo Whistle? Twitter?) — CANopen 인터페이스로 통일 (EtherCAT 사용 X)
