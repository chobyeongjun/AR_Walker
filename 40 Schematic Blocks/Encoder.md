---
aliases: [Encoder Block]
tags: [type/block, phase/A, domain/sensor]
type: schematic_block
phase: A
status: removed
kicad_sheet: encoder.kicad_sch
---

# Encoder

## 목적

케이블 풀리 회전각 절대 측정 (2채널) — [[AS5048A]] × 2.

## 핵심 부품

- [[AS5048A]] × 2 (TSSOP-14)
- SPI MISO 풀업 × 2 (10kΩ)
- 3.3V 바이패스 100nF × 2
- JST-GH 6pin × 2 (각 엔코더 연결선)

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| SPI3_SCK | PB3 | |
| SPI3_MISO | PB4 | |
| SPI3_MOSI | PB5 | |
| /CS1 | PA15 | 엔코더 1 |
| /CS2 | PB6 | 엔코더 2 |

## 외부 회로

- 3.3V 입력, GND 공유
- MISO 풀업 (3.3V ~ MISO)
- 바이패스 100nF, VDD 근처

## 자석 배치 (물리 조립)

- 다이아메트럴 자화 (N극-S극 수평)
- IC 중앙 위 1~2mm 거리
- 풀리 회전축과 정렬

## 관련 부품

- [[AS5048A]]

## 데이터시트

- [ ] ams AS5048A 데이터시트
- [ ] Adafruit AS5048A breakout schematic

## 작업 체크리스트

- [ ] 심볼 · 풋프린트 확인
- [ ] sheet 그리기
- [ ] ERC 통과
