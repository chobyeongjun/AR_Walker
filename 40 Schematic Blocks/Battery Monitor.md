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

[[INA228]] × **2** 로 배터리 팩 전압·전류·전력·쿨롱 측정 → [[Battery SoC Estimation|SoC 추정]].

Legacy HANDOFF 에서 **INA228 × 2** 확정 (직렬 배터리 개별 측정). 24V 단일 시스템이면 2번째는 DNP.

## 핵심 부품

- [[INA228]] VSSOP-10 × **2**
- Shunt 1mΩ 2W (Susumu CSS2H, 2512) **× 2** (각 팩 입력 라인 직렬)
- I²C 풀업 4.7kΩ × 2
- 3.3V 바이패스 (100nF + 1µF) × 2
- JST-GH 2pin (각 배터리 팩 NTC 입력) × 2
- NTC 풀업 10kΩ × 2

## 토폴로지 (48V 직렬 시)

```
Battery Pack 1 (24V)           Battery Pack 2 (24V)
    (+)── [shunt1 1mΩ] ──┬──► Battery Pack 2 (+) ── [shunt2 1mΩ] ──► Bus
                         │
                      INA228 #1 (I²C 0x40)
                         │
                      INA228 #2 (I²C 0x41)  — shunt2 양단 측정

   전력 = 셀 팩 1 + 셀 팩 2 (개별 + 합산)
```

## 24V 단일 시스템 시

```
Battery (+) ── [shunt 1mΩ] ── Bus
                    │
                 INA228 #1 (I²C 0x40)
                 INA228 #2 풋프린트 DNP (미실장)
```

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| I²C1_SCL | PB8 | 공유 |
| I²C1_SDA | PB9 | 공유 |
| INA228_ALERT #1 | PE5 | EXTI (과전류 알림) |
| INA228_ALERT #2 | PE6 | EXTI (옵션) |
| BATT_NTC1_ADC | PC4 | ADC1_IN4 |
| BATT_NTC2_ADC | PC5 | ADC1_IN5 (옵션) |

## 외부 회로 (팩당)

- Shunt 저항: [[Motor Power]] 블록의 입력 라인에 배치 (P-MOS·인러시 통과 후)
- INA228 IN+/IN-: shunt 양단 직결 (Kelvin 연결 권장 — 트레이스 분리)
- I²C 풀업은 [[MCU Core]] 의 I²C 버스 공유

## 관련 개념

- [[Battery SoC Estimation]]

## 관련 부품

- [[INA228]] ⭐ × 2

## 데이터시트

- [ ] TI INA228 데이터시트

## 작업 체크리스트

- [ ] shunt 저항 정격 확인 (30A × 30A × 1mΩ = 0.9W → 2W 충분)
- [ ] shunt 물리 위치 결정 (팩별 독립)
- [ ] Kelvin 연결 트레이스 설계
- [ ] I²C 주소 핀 설정 (ADDR0 = GND 또는 VS)
- [ ] sheet 그리기
