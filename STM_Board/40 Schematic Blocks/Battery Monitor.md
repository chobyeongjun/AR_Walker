---
aliases: [Battery Monitor Block]
tags: [type/block, phase/A, domain/power, domain/sensor]
type: schematic_block
phase: A
status: pending
kicad_sheet: battery_monitor.kicad_sch
---

# Battery Monitor

## 목적

[[INA228]] 로 배터리 팩 전압·전류·전력·쿨롱 측정 → [[Battery SoC Estimation|SoC 추정]].

## 핵심 부품

- [[INA228]] VSSOP-10
- Shunt 1mΩ 2W (Susumu CSS2H, 2512) — 입력 +V 라인 직렬
- I²C 풀업 4.7kΩ × 2
- 3.3V 바이패스 (100nF + 1µF)
- JST-GH 2pin (배터리 NTC 입력)
- NTC 풀업 10kΩ

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| I²C1_SCL | PB8 | |
| I²C1_SDA | PB9 | |
| INA228_ALERT | PE5 | EXTI (선택) |
| BATT_NTC_ADC | PC4 | ADC1_IN4 |

## 외부 회로

- Shunt 저항은 [[Motor Power]] 블록의 입력 라인에 배치 (eFuse 통과 후)
- INA228 IN+/IN- 는 shunt 양단에 직결
- I²C 풀업 ([[MCU Core]] 의 I²C 버스 공유)

## 관련 개념

- [[Battery SoC Estimation]]

## 관련 부품

- [[INA228]]

## 데이터시트

- [ ] TI INA228 데이터시트

## 작업 체크리스트

- [ ] shunt 저항 정격 (2W, 30A × 30A × 1mΩ = 0.9W 수용) 확인
- [ ] shunt 위치는 [[Motor Power]] 와 협의
- [ ] sheet 그리기
