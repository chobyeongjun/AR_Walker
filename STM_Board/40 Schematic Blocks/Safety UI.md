---
aliases: [Safety UI Block]
tags: [type/block, phase/A, domain/safety]
type: schematic_block
phase: A
status: pending
kicad_sheet: safety_ui.kicad_sch
---

# Safety UI

## 목적

E-stop (이중 안전) + RGB 상태 LED + 버튼 + sync LED.

## 핵심 부품

- JST-GH 2pin (E-stop NC 접점 입력)
- 10 kΩ 풀업 + 10 nF 디바운스 + 1 kΩ 직렬
- [[74LVC1G08]] AND 게이트 (SC-70-5)
- RGB LED (common cathode 또는 APA102)
- 적색 E-stop 상태 LED (0603)
- Sync LED (파란 0603)
- USER 버튼 (tactile, 6×6)
- RESET 버튼 (tactile)
- (옵션) 74LVC1G373 래치 + reset 버튼

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| ESTOP_EXTI | PC13 | EXTI 인터럽트 |
| MOTOR_ENABLE | PE3 | → AND gate 입력 |
| SYNC_LED | PE4 | TIM PWM 가능 |
| RGB_R | PA8 | TIM1_CH1 |
| RGB_G | PA9 | TIM1_CH2 |
| RGB_B | PA10 | TIM1_CH3 |
| USER_BTN | PB10 | 풀업 + EXTI |

## 회로 핵심

### E-stop 이중 차단 (자세한 설명: [[E-stop Dual Safety]])

```
ESTOP_NC ── pull-up + 디바운스 ─┬─► MCU EXTI (SW)
                                 │
                                 ├─► AND gate in_A
                                 │
MCU motor_enable ────────────────┴─► AND gate in_B
                                 │
                                 ▼
                          motor_enable_final (→ 모터 드라이버)
```

## 관련 개념

- [[E-stop Dual Safety]]

## 관련 부품

- [[74LVC1G08]]

## 작업 체크리스트

- [ ] AND gate 배치 (motor_enable_final 라인 가까이)
- [ ] LED 드롭 저항 계산 (3.3V 기준)
- [ ] 버튼 위치 고려 (케이스 설계)
- [ ] sheet 그리기
