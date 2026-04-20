---
aliases: [H723VGT6, STM32H7, 메인 MCU]
tags: [type/component, domain/mcu, phase/A, status/decided]
type: component
part_number: STM32H723VGT6
package: LQFP100 (14x14)
manufacturer: ST
voltage_range: 1.62-3.6V
interface: "CAN-FD, USB HS, SPI, I²C, UART, SDMMC, ADC"
phase: A
status: decided
---

# STM32H723VGT6

## 한 줄 요약

Cortex-M7 550MHz + USB HS 내장 PHY + CAN-FD + HRTIM — wearable exosuit 에 맞는 컴팩트·고성능 MCU.

## 사양

| 항목 | 값 |
|---|---|
| CPU | Cortex-M7 @ 550 MHz, DP-FPU |
| Flash | 1 MB (single bank) |
| RAM | 564 KB (DTCM 128K + ITCM 64K + AXI 320K) |
| 패키지 | LQFP100 (14 × 14 mm), 0.5mm 피치 |
| USB HS | 내장 PHY ⭐ |
| CAN-FD | 2개 (FDCAN1 사용) |
| SDMMC | 2 (4-bit 모드) |
| Timer | HRTIM + 다수 GP timer |
| ADC | 3 × 16-bit |
| 소비전력 | ~120 mA @ 550MHz |
| 가격 | ~$8~12 |

## 왜 선택했나

→ 자세한 비교는 [[BGA vs LQFP]] + [[Decisions Log]] 참조

**핵심 근거:**
1. **컴팩트** — 14×14mm, 자가 납땜 가능한 가장 작은 H7 (BGA 제외)
2. **USB HS 내장 PHY** — 외부 ULPI PHY 없이 480Mbps (vs H743V는 외부 PHY 필요)
3. **HRTIM + CAN-FD** — 미래 모터 직결도 가능 (현재는 [[TCAN1462]] 사용)
4. **Flash 1MB + RAM 564KB** — 펌웨어 (제어·BLE·SD·로깅) + 여유분 충분
5. **Nucleo-H723ZG 와 코드 호환** — 라이브러리·예제 재활용

## 연결되는 블록

- [[MCU Core]] — 핵심 블록
- [[Comms]] — FDCAN1, UART
- [[Loadcell Amp]] — SPI1 + DMA
- [[Encoder]] — SPI3
- [[Storage]] — SDMMC1
- [[USB Debug]] — OTG_HS
- [[Battery Monitor]] — I²C
- [[Wireless]] — UART (ESP32)
- [[Safety UI]] — GPIO EXTI

## 관련 개념

- [[BGA vs LQFP]]
- [[Hardware Sync for Jetson]]

## 데이터시트 / 레퍼런스

- [ ] STM32H723xx 데이터시트 PDF → `50 References/Datasheets/`
- [ ] ST AN5436 "Hardware checklist for STM32H7" PDF
- [ ] ST AN2867 "Oscillator design guide" PDF
- [ ] Nucleo-H723ZG schematic PDF (참고 보드)

## 주요 핀 (초안)

| 기능 | 핀 |
|---|---|
| HSE | PH0/PH1 (25 MHz crystal) |
| LSE | PC14/PC15 (32.768 kHz, 옵션) |
| SWD | PA13 (SWDIO) / PA14 (SWCLK) |
| USB_HS_DM/DP | PA11 / PA12 |
| FDCAN1 | PD0 (RX) / PD1 (TX) |
| SDMMC1 | PC8-12 (D0-D3, CK, CMD) |
| SPI1 (ADS131M04) | PA5/6/7 + CS |
| SPI3 (AS5048A) | PB3/4/5 + 2× CS |
| I²C (INA228) | PB8/PB9 |
| UART (Jetson) | PA0 (TX) / PA1 (RX) |
| UART (EBIMU) | PC10 / PC11 |
| UART (ESP32) | PD5 / PD6 |

> 최종 핀맵은 CubeMX `.ioc` 에서 확정.

## 주의사항

- **VCAP 핀**: 2.2µF 세라믹 × 2 (내부 LDO 안정화) — 생략 시 부팅 실패
- **BOOT0**: 점퍼 + 풀다운. DFU 모드 진입에 필수
- **VDDA/VREF+**: 별도 ferrite + LDO 분리 ([[Logic Power]])
- **NRST**: 100nF + 10kΩ 풀업 + 버튼
- LQFP100 핀피치 0.5mm — 스텐실 필수
