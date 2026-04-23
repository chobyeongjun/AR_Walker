---
aliases: [Logic Power Block]
tags: [type/block, phase/A, domain/power]
type: schematic_block
phase: A
status: pending
kicad_sheet: logic_power.kicad_sch
---

# Logic Power

## 목적

모터 전원 → 5V → 3.3V (디지털/아날로그 분리) — MCU·센서·통신 IC 공급.

## 핵심 부품

- Ferrite bead (모터 라인 → buck 입력 격리)
- [[TPS62933]] Buck (SOT583, 2.5 MHz)
- 2.2 µH 인덕터 (XAL4040-222, 4×4mm)
- 버크 입력 캡 22 µF + 1 µF (V 정격은 Phase B)
- 버크 출력 캡 22 µF × 2 + 100 nF
- 피드백 저항 분압 (5V 출력)
- [[TLV75533]] × 2 (SOT-23-5)
- 아날로그 ferrite (BLM18PG471) + 1µF
- 각 LDO 출력 1µF + 100nF
- 전원 LED (녹 3.3V)

## 전원 토폴로지

```
Motor Rail (24~48V)
    │
    ├── ferrite bead ─► [[TPS62933]] ──► 5V
    │                                     │
    │                                     ├── TLV75533 #1 ──► 3V3_D (digital)
    │                                     │                     │
    │                                     │                     ├── MCU VDD
    │                                     │                     ├── 통신 IC VCC
    │                                     │                     └── ESP32, microSD, USB
    │                                     │
    │                                     └── ferrite ─► TLV75533 #2 ──► 3V3_A (analog)
    │                                                                     │
    │                                                                     ├── ADS131M04 AVDD
    │                                                                     ├── REF5025 VIN
    │                                                                     └── MCU VDDA/VREF+
    │
    └── (Phase B motor power)
```

## 핀맵 (MCU 측)

전원 블록 자체는 MCU 핀 사용 없음. (옵션으로 5V_PG, 3V3_PG 같은 파워굿 → MCU GPIO)

## 관련 개념

- [[Motor Power Isolation]]
- [[Loadcell Amplifier Design]]

## 관련 부품

- [[TPS62933]]
- [[TLV75533]]

## 데이터시트

- [ ] TI TPS62933 데이터시트
- [ ] TI TLV75533 데이터시트
- [ ] TI WEBENCH TPS62933 산출물

## 작업 체크리스트

- [ ] 5V 출력 피드백 저항 계산 (WEBENCH)
- [ ] 입력 캡 V 정격 (Phase B 에서 확정)
- [ ] ferrite bead 임피던스 선택 (BLM18PG 시리즈)
- [ ] sheet 그리기
