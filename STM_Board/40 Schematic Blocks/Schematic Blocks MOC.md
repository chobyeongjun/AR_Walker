---
aliases: [Schematic Blocks]
tags: [moc, blocks]
---

# Schematic Blocks MOC

KiCad hierarchical sheet 단위의 작업 노트. 각 블록 = 별도 `.kicad_sch` 파일.

## Phase A (모터 무관 — 지금 작업)

우선순위 순:

1. [[Logic Power]] — 5V→3.3V LDO 기반 (다른 블록 전제)
2. [[MCU Core]] — STM32H723VGT6 + HSE/LSE/SWD + 디커플링
3. [[Loadcell Amp]] — **사용자 우선** · ADS131M04 + REF5025
4. [[Encoder]] — AS5048A × 2
5. [[Comms]] — CAN-FD + Jetson UART + SYNC + EBIMU
6. [[Wireless]] — ESP32-C3-MINI-1U + IPEX
7. [[Storage]] — microSD SDMMC
8. [[USB Debug]] — USB-C + ESD
9. [[Safety UI]] — E-stop + LED + 버튼
10. [[Battery Monitor]] — INA228 + shunt + NTC

## Phase B (모터 결정 후)

11. [[Motor Power]] — DC입력 + eFuse + 큰 캡 + 모터 출력
