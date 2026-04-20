---
aliases: [Storage Block, SD Card]
tags: [type/block, phase/A, domain/storage]
type: schematic_block
phase: A
status: pending
kicad_sheet: storage.kicad_sch
---

# Storage

## 목적

[[DM3AT microSD]] push-push 슬롯 + SDMMC 4-bit — 온보드 데이터 로깅.

## 핵심 부품

- [[DM3AT microSD]] 슬롯
- 50kΩ 풀업 × 5 (CMD, D0~D3)
- 3.3V 바이패스 (100nF + 10µF 1206)
- ESD 다이오드 어레이 (선택, USBLC6-4 등)

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| SDMMC1_CMD | PD2 | |
| SDMMC1_CLK | PC12 | |
| SDMMC1_D0 | PC8 | |
| SDMMC1_D1 | PC9 | |
| SDMMC1_D2 | PC10 | EBIMU UART4_RX 와 충돌 — 재검토 |
| SDMMC1_D3 | PC11 | |
| CARD_DETECT | PE2 (EXTI) | |

## 주의

UART4 (EBIMU) 와 SDMMC1 D2/D3 핀 충돌 가능성 — CubeMX 에서 alt mapping 찾기.
대안: SDMMC2 로 이동 (PB3/4/14/15 등).

## 관련 부품

- [[DM3AT microSD]]

## 데이터시트

- [ ] Hirose DM3AT 데이터시트
- [ ] ST AN5200 SDMMC 가이드

## 작업 체크리스트

- [ ] 핀 충돌 해결 (CubeMX)
- [ ] 심볼·풋프린트 확인
- [ ] sheet 그리기
