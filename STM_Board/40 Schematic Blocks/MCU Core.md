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

[[STM32H743VIT6]] 메인 MCU + 클럭 + 전원 디커플링 + SWD 디버그.

## 핵심 부품

- [[STM32H743VIT6]] LQFP100
- HSE 25 MHz crystal + 22pF × 2 + **22Ω 직렬 저항** (legacy W4)
- LSE 32.768 kHz crystal + 12pF × 2 (옵션, RTC)
- **VCAP 2.2µF × 2** (VCAP_1 Pin 33, **VCAP_2 Pin 57** — legacy C4)
- NRST 100nF + 10kΩ 풀업 + tactile 버튼
- BOOT0 10kΩ 풀다운 + 점퍼 (DFU)
- SWD 10-pin Cortex Debug 커넥터 (SWDIO/SWCLK/SWO/NRST/3V3/GND)

## ⚠️ Legacy Review 에서 발견된 치명 이슈 (C4~C7, W2~W7)

반드시 회로에 반영해야 할 항목들:

### C4 — VCAP_2 누락 금지 (LQFP100 은 VCAP 2개)

```
VCAP_1 (Pin 33) ─[2.2µF X5R/6.3V]─ GND
VCAP_2 (Pin 57) ─[2.2µF X5R/6.3V]─ GND   ← 놓치면 MCU 미동작
```

### C5 — VREF+ 처리

```
VDDA (Pin 11) ──── VREF+ (Pin 21)
                    ├── 1 µF
                    └── 100 nF
```

VDDA 와 VREF+ 직결 후 별도 디커플링. 미처리 시 내장 ADC 전면 미동작.

### W2 — VDDA 필터 강화

```
3.3V ─[BLM18PG601 600Ω@100MHz]─ VDDA
                                  ├── 10 µF X5R
                                  ├── 1 µF
                                  └── 100 nF
```

페라이트 비드로 디지털 노이즈 차단 (ADC SNR 결정).

### W4 — HSE 크리스털 직렬 저항

```
OSC_IN  ─[22Ω]─┐
               │ 25 MHz crystal
OSC_OUT ───────┤
               │
            22pF × 2 → GND
```

22Ω 이 없으면 고조파 ringing 으로 EMI 문제 + 클럭 지터.

### W3 — PC13 (RTC 도메인) 주의

PC13 는 **max 3mA** — 직접 부하 (LED) 구동 금지. 필요 시 100Ω 직렬 또는 다른 GPIO 로 재배정.

### W6 — BOOT0 처리

```
BOOT0 ─[10kΩ]─ GND  (평시 flash 부트)
       └──── 점퍼/버튼 to 3.3V  (DFU 모드 진입)
```

### W7 — NRST 필터

```
NRST ─[100nF]─ GND
      └───── tactile 버튼 → GND
```

저항 불필요 (내부 풀업). 100nF 만.

## 추가 디커플링

- VDD (Pin 10, 28, 50, 62, 75, 100): 각 100nF + 공유 4.7µF
- VSS: 각 핀 0Ω 아닌 직결 GND 플레인

## 핀 할당 (상태별)

> **🚩 대부분 CubeMX 확정 필요.** 상세 테이블은 [[STM32H743VIT6]] 참고.

✅ 확정 (ST DS12110 고정핀 또는 legacy):
- HSE PH0/PH1, LSE PC14/PC15
- SWDIO PA13, SWCLK PA14
- VCAP_1 Pin 33, VCAP_2 Pin 57, VDDA Pin 11, VREF+ Pin 21
- FDCAN1 PD0/PD1 (legacy W12)
- USB FS PA11/PA12 (고정)

🚩 추측 (CubeMX 에서 충돌 검사 · 재배정 예상):
- SPI1/SPI3, I²C1, UART2/3/4, VBUS_SENSE, TIM1, ESTOP, MOTOR_ENABLE
- UART4 (EBIMU) ↔ SDMMC1 D2/D3 충돌 가능성 — **재배정 필요**
- SWD SWO (PB3) ↔ SPI3 SCK (PB3) 잠재 충돌

> **Hardware SYNC** 사용자 확정 (v3.10) — SYNC_OUT (PB14), SYNC_IN (PB15) 🚩

## 관련 개념

- [[BGA vs LQFP]] — LQFP100 선택
- [[GND Bounce Protection]] — PCB 레이아웃 규칙
- [[Hardware Sync for Jetson]] — 사용자 확정 (v3.10)

## 관련 결정

- [[RECONCILIATION]] — H723 → H743 변경
- [[Decisions Log]]

## 데이터시트

- [ ] STM32H743xx 데이터시트 PDF
- [ ] ST AN5436 Hardware checklist
- [ ] ST AN2867 Oscillator design guide
- [ ] Nucleo-H743ZI schematic (레퍼런스)

## 작업 체크리스트

- [ ] reference schematic 수집
- [ ] KiCad 심볼 확인 (`MCU_ST_STM32H7` 라이브러리)
- [ ] LQFP100 풋프린트 확인
- [ ] VCAP_1 + **VCAP_2** 2.2µF 배치 확인
- [ ] VREF+ VDDA 연결 확인
- [ ] VDDA 페라이트 비드 (BLM18PG601) 확인
- [ ] HSE 22Ω 직렬 저항 확인
- [ ] hierarchical sheet 그리기
- [ ] ERC 통과
- [ ] CubeMX `.ioc` 와 핀 대조
