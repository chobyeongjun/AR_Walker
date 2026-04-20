---
aliases: [H743, H743VIT6, STM32H7, 메인 MCU]
tags: [type/component, domain/mcu, phase/A, status/decided]
type: component
part_number: STM32H743VIT6
package: LQFP100 (14x14)
manufacturer: ST
voltage_range: 1.62-3.6V
interface: "CAN-FD, USB FS/HS(외부PHY), SPI, I²C, UART, SDMMC, ADC, Ethernet"
phase: A
status: decided
---

# STM32H743VIT6

> **확정 MCU** — Cortex-M7 480 MHz + 2MB Flash + 1MB RAM, legacy review 에서 검증 완료.
> 내 원안 [[STM32H723VGT6]] 에서 변경 (이유: [[RECONCILIATION]] §🔴)

## 한 줄 요약

케이블 드리븐 exosuit 제어용 고성능 MCU. Flash/RAM 여유 + dual-bank OTA + 성숙한 HAL + Nucleo-H743ZI 레퍼런스.

## 사양

| 항목 | 값 |
|---|---|
| CPU | Cortex-M7 @ 480 MHz, DP-FPU |
| Flash | **2 MB** (dual bank) |
| RAM 총량 | **1 MB** |
| └ DTCM | 128 KB (zero-wait) |
| └ ITCM | 64 KB |
| └ AXI SRAM | 512 KB |
| └ SRAM123 | 288 KB |
| └ SRAM4 | 64 KB |
| CAN-FD (FDCAN) | 2개 — FDCAN1 (PD0/1) + FDCAN2 (PB12/13) |
| USB | OTG_FS 내장, OTG_HS 는 외부 ULPI PHY 필요 (우리는 FS 만 사용) |
| SDMMC | 2 (4-bit 모드 가능) |
| Ethernet | RMII MAC 내장 (우리 미사용) |
| HRTIM | 1 (모터 PWM 확장 시) |
| TIM | 다수 (제어 인터럽트용) |
| ADC | 3 × 16-bit |
| I²C | 4 |
| SPI | 6 |
| UART | 8 |
| 패키지 | LQFP100 (14 × 14mm, 0.5mm 피치) |
| VCAP 핀 | **2개: Pin 33 (VCAP_1), Pin 57 (VCAP_2)** ⚠️ |
| VDDA / VREF+ | 별도 핀 |
| 온도 등급 | -40~85°C / -40~125°C (suffix) |
| 가격 | ~$12~16 |

## 왜 H723 대신 H743

→ 자세한 비교·결정 근거: [[RECONCILIATION]]

**H743 유리:**
- **Flash 2 MB + RAM 1 MB** (H723 대비 2× / 2×) — FreeRTOS + 제어 + BLE + SD + 미래 CNN inference 여유
- **Dual-bank flash** — 안전 OTA 업데이트 (H723 single bank)
- **ST HAL 가장 성숙** (2017 출시, 6+ 년 축적)
- **Nucleo-H743ZI 개발보드** — Phase A 펌웨어 선개발 가능 (legacy plan)

**H723 의 유일한 장점** (USB HS 내장 PHY) 우리 use case 에서 **무의미** — USB 는 debug/DFU 만 쓰므로 FS (12Mbps) 로 충분.

## ⚠️ 회로 설계 필수 사항 (legacy C4/C5/W2/W4/W5/W6/W7 반영)

### VCAP 핀 2개 (내부 1.2V LDO 안정화)

**절대 누락 금지** — LQFP100 은 VCAP 가 2개. 둘 다 독립 2.2µF 필요:

```
VCAP_1 (Pin 33) ─[2.2µF X5R/6.3V]─ GND
VCAP_2 (Pin 57) ─[2.2µF X5R/6.3V]─ GND   ← 이거 놓치면 MCU 미동작
```

### VDDA + VREF+

```
3.3V ─[BLM18PG601 600Ω@100MHz]─ VDDA (Pin 11)
                                  ├── 10 µF X5R
                                  ├── 1 µF
                                  └── 100 nF

VDDA ──────────────────── VREF+ (Pin 21)
                           ├── 1 µF
                           └── 100 nF
```

### HSE 25 MHz 크리스털 (USB·Ethernet 정확도)

```
OSC_IN  ─[22Ω]─┐
               │ 크리스털 25 MHz
OSC_OUT ───────┤
               │
            22pF × 2 로드 캡 (GND)
```
- 22Ω 직렬 저항: 고조파 억제 ([[RECONCILIATION]] W4)
- 크리스털 아래 Layer 2 GND 비아 없음 (가드링만)

### NRST + BOOT0 + SWD

```
NRST  ─[100nF]─ GND  +  tactile 버튼 to GND
BOOT0 ─[10 kΩ]─ GND  +  점퍼 to 3.3V (DFU 진입용)
SWD   = PA13 (SWDIO) + PA14 (SWCLK) + SWO + GND + 3V3 + NRST → Cortex Debug 10-pin
```

## 핀 할당 (상태: 대부분 🚩 CubeMX 확정 필요)

> **⚠️ 중요:** 아래 테이블은 legacy 에서 확정된 것 (✅) 과 내가 AF 매핑 기반 추측 (🚩) 이 섞여 있음.
> 🚩 표시된 핀은 **STM32CubeMX `.ioc` 에서 실제 충돌 검사 후 확정** 해야 함.
> CubeMX 설명: [[Citations & Sources]] — "불확실·미검증" 섹션.

| 기능 | 핀 (제안) | 상태 | 출처 |
|---|---|---|---|
| HSE | PH0 / PH1 | ✅ | ST DS12110 (H743 LQFP100 고정핀) |
| LSE | PC14 / PC15 | ✅ | ST DS12110 (고정핀, 옵션) |
| SWD SWDIO | PA13 | ✅ | ST DS12110 (고정핀) |
| SWD SWCLK | PA14 | ✅ | ST DS12110 (고정핀) |
| VCAP_1 | Pin 33 | ✅ | legacy C4 |
| VCAP_2 | Pin 57 | ✅ | legacy C4 |
| VDDA | Pin 11 | ✅ | legacy W2 |
| VREF+ | Pin 21 | ✅ | legacy C5 |
| FDCAN1 RX/TX | PD0 / PD1 | ✅ | legacy W12 |
| SDMMC1 | PC8-12, PD2 | 🚩 | AF 매핑 추측 |
| SPI1 (ADS131M04) | PA5/6/7 + CS PA4 | 🚩 | AF 매핑 추측 |
| SPI3 (AS5048A × 2) | PB3/4/5 + 2× CS | 🚩 | AF 매핑 추측 + SWD SWO 와 PB3 공유 주의 |
| I²C1 (INA228) | PB8 / PB9 | 🚩 | AF 매핑 추측 |
| UART2 (Jetson) | PA2 / PA3 | 🚩 | AF 매핑 추측 |
| UART3 (ESP32) | PD8 / PD9 | 🚩 | AF 매핑 추측 |
| UART4 (EBIMU) | PC10 / PC11 | 🚩 | **SDMMC1 D2/D3 와 충돌 — CubeMX 에서 재배정** |
| USB FS | PA11 / PA12 | ✅ | ST DS12110 (고정핀) |
| VBUS_SENSE | PA9 | 🚩 | ADC 매핑 추측 |
| TIM1 PWM (RGB) | PA8/9/10 | 🚩 | AF 매핑 추측 + PA9 VBUS와 공유 X |
| ESTOP EXTI | PC13 | 🚩 | PC13 3mA 한계 — legacy W3 |
| MOTOR_ENABLE | PE3 | 🚩 | 임의 선택 |

> **Hardware SYNC GPIO 제거됨** (v3.9) — [[Hardware Sync for Jetson]] 참고.

## 연결되는 블록

- [[MCU Core]] — 메인 블록
- [[Comms]] — FDCAN1, UART2/3/4
- [[Loadcell Amp]] — SPI1 + DMA
- [[Encoder]] — SPI3
- [[Storage]] — SDMMC1
- [[USB Debug]] — OTG_FS
- [[Battery Monitor]] — I²C1
- [[Wireless]] — UART3
- [[Safety UI]] — GPIO EXTI

## 관련 개념

- [[BGA vs LQFP]] — LQFP100 선택 근거
- [[Modular Phase A-B Strategy]]
- [[Hardware Sync for Jetson]] — 제거됨 (v3.9)

## 데이터시트 / 레퍼런스

- [ ] STM32H743xx 데이터시트 PDF → `50 References/Datasheets/`
- [ ] ST AN5436 "Hardware checklist for STM32H7" PDF
- [ ] ST AN2867 "Oscillator design guide" PDF
- [ ] **Nucleo-H743ZI schematic PDF** (Phase A 펌웨어 선개발 보드)
- [ ] ST AN4879 "USB HS PHY design guide" (우리 미사용, 참고)

## 주의사항

- **VCAP_2 (Pin 57) 놓치면 MCU 미동작** — LQFP100 특유, legacy C4
- **VDDA 필터 필수** — 페라이트 비드 없이 3.3V 직결 시 ADC 노이즈 심각
- LQFP100 0.5mm 핀피치 — 외주 SMT 시 스텐실·AOI 필수
- ST HAL 업데이트 주기 확인 (CubeMX 에서 자동)
