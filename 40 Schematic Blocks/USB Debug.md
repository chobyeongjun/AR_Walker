---
aliases: [USB Debug Block]
tags: [type/block, phase/A, domain/comm]
type: schematic_block
phase: A
status: pending
kicad_sheet: usb_debug.kicad_sch
---

# USB Debug

## 목적

USB-C 커넥터 → MCU OTG_HS (내장 PHY) — DFU 부트로더, CDC 디버그, VCP.

## 핵심 부품

- USB-C 16pin 리셉터클 (GCT USB4105 또는 동급)
- [[TPD4S012]] ESD 어레이
- CC1/CC2 5.1kΩ 풀다운 (sink mode)
- VBUS sensing 저항 분압 (2:1) → MCU VBUS_SENSE
- VBUS TVS + ferrite (옵션)

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| OTG_HS_DM | PA11 | |
| OTG_HS_DP | PA12 | |
| USB_VBUS_SENSE | PA9 | ADC or GPIO |

## 배치 규칙

- USB-C 커넥터는 **보드 모서리** 에 배치 (케이블 접근)
- D+/D- 트레이스 length matching (±0.1mm), 90Ω 차동 임피던스
- ESD 는 커넥터 바로 뒤
- CC 풀다운 가까이 배치

## 관련 부품

- [[TPD4S012]]

## 데이터시트

- [ ] USB-C 커넥터 데이터시트 (GCT USB4105)
- [ ] TI TPD4S012
- [ ] Sparkfun USB-C breakout schematic (참고)

## 작업 체크리스트

- [ ] 커넥터 풋프린트
- [ ] ESD 배치 규칙 확인
- [ ] sheet 그리기
