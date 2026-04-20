---
aliases: [MCU Core]
tags: [type/block, phase/A, domain/mcu]
type: schematic_block
phase: A
status: pending
kicad_sheet: mcu_core.kicad_sch
---

# MCU Core

## 목적

[[STM32H723VGT6]] 메인 MCU + 클럭 + 전원 디커플링 + SWD 디버그.

## 핵심 부품

- [[STM32H723VGT6]]
- HSE 25 MHz crystal + 2× 22pF
- LSE 32.768 kHz crystal + 2× 12pF (옵션, RTC)
- VCAP 2.2 µF × 2
- NRST 100nF + 10kΩ + 버튼
- BOOT0 10kΩ 풀다운 + 점퍼
- SWD 10-pin Cortex Debug 커넥터

## 외부 회로 핵심

### 전원 디커플링

- VDD: 4.7 µF + (100 nF × 8핀 각각)
- VDDA: 1 µF + 100 nF + ferrite 입력 ([[Loadcell Amplifier Design]])
- VCAP1/VCAP2: 2.2 µF 세라믹 X7R (**절대 생략 금지**)

### 클럭

- HSE 25 MHz → USB HS · Ethernet 정확도 유지
- 22 pF 로드 캡 (ST AN2867 계산)
- 트레이스 짧게 (크리스털 → MCU 10mm 이내)

### SWD + Trace

- SWDIO (PA13), SWCLK (PA14), SWO (PB3 또는 유사) → Cortex Debug 10-pin
- NRST 도 connector 포함

## 관련 개념

- [[BGA vs LQFP]]
- [[PCB Stackup 6-layer]]

## 데이터시트 / 레퍼런스

- [ ] STM32H723xx 데이터시트
- [ ] ST AN5436 Hardware checklist
- [ ] ST AN2867 Oscillator design guide
- [ ] Nucleo-H723ZG schematic (레퍼런스)

## 작업 체크리스트

- [ ] reference schematic 수집
- [ ] KiCad 심볼 확인 (`MCU_ST_STM32H7` 라이브러리)
- [ ] LQFP100 풋프린트 확인
- [ ] hierarchical sheet 그리기
- [ ] ERC 통과
- [ ] CubeMX `.ioc` 와 핀 대조
