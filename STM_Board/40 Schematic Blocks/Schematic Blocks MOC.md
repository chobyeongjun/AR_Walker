---
aliases: [Schematic Blocks]
tags: [moc, blocks]
---

# Schematic Blocks MOC

KiCad hierarchical sheet 단위 작업 노트. 각 블록 = 별도 `.kicad_sch` 파일.

## Phase A — 모터 무관 (지금 작업)

우선순위 순:

1. [[Logic Power]] — 5V→3.3V LDO (다른 블록 전제)
2. [[MCU Core]] — [[STM32H743VIT6]] + HSE/LSE/SWD + 디커플링 (VCAP_2 + VREF+ 주의)
3. [[Loadcell Amp]] ⭐ 사용자 우선 — [[ADS131M04]] + [[REF5025]]
4. [[Encoder]] — [[AS5048A]] × 2
5. [[Comms]] — **[[ISO1050]] 격리 CAN** + Jetson UART + SYNC + EBIMU
6. [[Wireless]] — [[ESP32-C3-MINI-1U]] + IPEX
7. [[Storage]] — [[DM3AT microSD]] SDMMC
8. [[USB Debug]] — USB-C + [[TPD4S012]] ESD
9. [[Safety UI]] — E-stop + [[74LVC1G08]] AND gate + LED
10. [[Battery Monitor]] — [[INA228]] × 2 + shunt + NTC

## Phase B — 모터 결정 후

11. [[Motor Power]] — DC 입력 + 역전압 + 인러시 + 버스 보호 + 브레이크 + 모터별 로컬 + 로직 분기
    - [[Regen Energy Protection]]
    - [[Brake Resistor Circuit]]
    - [[Inrush Current Limiting]]
    - [[Isolated CAN]] 와 연계 (PGND 공유)

## v3.5 주요 업데이트

- [[Comms]] 블록 — [[TCAN1462]] → [[ISO1050]] 격리로 전환
- [[MCU Core]] 블록 — H723 → H743, VCAP_2/VREF+/HSE 22Ω 추가
- [[Motor Power]] 블록 — legacy 의 완전한 보호 아키텍처 채택 (재생·브레이크·인러시)
