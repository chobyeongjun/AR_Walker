---
aliases: [Decisions Log, Decisions]
tags: [project, decisions]
---

# Decisions Log

시간순 기록. 각 결정은 영향받는 노트로 `[[wikilink]]` 연결.

## 2026-04-17

| 항목 | 결정 | 관련 |
|---|---|---|
| 디렉토리 | `AR_Walker/STM_Board/` + `~/stm_board` 심링크 | |
| 모터 모델 | 보류 (Phase 5) — Elmo 가정 | [[Open Questions]] |
| 모터 인터페이스 | CANopen | [[TCAN1462]], [[Comms]] |
| 로드셀 ADC | HX711 → [[ADS131M04]] | [[Loadcell Amplifier Design]] |
| 로깅 | 온보드 microSD SDMMC 4-bit | [[Storage]] |
| CAN 버스 | 1개 | [[Comms]] |
| IMU | EBIMU 무선 (수신기 UART) | [[Goals & Requirements]] |
| EtherCAT | 사용 안 함 | |
| DMA | 고속 주변기기 전부 매핑 | [[MCU Core]] |
| 무선 제어 | [[ESP32-C3-MINI-1U]] (IPEX 외부 안테나) | [[Wireless]] |
| Jetson 통신 | UART (DMA) + [[Hardware Sync for Jetson\|HW SYNC GPIO]] | |
| 컴팩트 필수 | wearable → layer ↑ 로 사이즈 ↓ | [[PCB Stackup 6-layer]] |
| **MCU** | **[[STM32H723VGT6]] (LQFP100)** | [[BGA vs LQFP]] |
| Ethernet | 완전 제거 | |
| USB-C 디버그 | 포함 | [[USB Debug]] |
| E-stop | 이중 안전 (SW + HW AND gate) | [[E-stop Dual Safety]] |
| 모터 전원 보드 통과 | 확정 | [[Motor Power Isolation]] |
| 모터 전류 (사용자) | 15A+ 연속 / 30A 피크 | [[Voltage-Current Scaling]] |
| DC 입력 커넥터 | XT60PW (30A) — Phase B 재확정 | |
| eFuse | [[LTC4368-1]] + [[IPB180N06S4]] | [[eFuse]] |
| PCB | 6-layer 50×50mm | [[PCB Stackup 6-layer]] |
| 외/내층 동박 | 2oz / 1oz | |
| 표면 처리 | ENIG | |
| 스택업 | SIG/GND/SIG/PWR/GND/SIG | |
| 배터리 모니터 | [[INA228]] + 1mΩ shunt + NTC | [[Battery SoC Estimation]] |
| SoC 알고리즘 | OCV + 쿨롱 카운팅 | [[Battery SoC Estimation]] |
| **모듈러 전략** | Phase A (로직, freeze) / Phase B (모터, 보류) | [[Modular Phase A-B Strategy]] |

## 2026-04-20

| 항목 | 결정 | 관련 |
|---|---|---|
| CubeMX 칩 | H723VGT6 진행 확정 (사용자 위임) | [[STM32H723VGT6]] |
| Vault 구조 | **Obsidian Second Brain 으로 재구성** | [[00 Index]] |
| 노트 단위 | atomic + wikilinks + MOC + YAML | |
| 다음 단계 | P2 KiCad 프로젝트 생성 | [[Phase Plan]] |

## 템플릿

새 결정은 [[Templates/Decision]] 을 복사해서 상세 기록.
간단한 결정은 위 표에 한 줄로 추가.
