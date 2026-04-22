---
aliases: [DM3AT, microSD slot]
tags: [type/component, domain/storage, phase/A, status/decided]
type: component
part_number: DM3AT-SF-PEJM5
package: SMD push-push
manufacturer: Hirose
voltage_range: 3.3V (card)
interface: SDMMC 4-bit
phase: A
status: decided
---

# DM3AT microSD

## 한 줄 요약

Push-push microSD 슬롯, 카드 detect 핀 — 로깅용 SDMMC 4-bit 인터페이스.

## 사양

- 소켓 면적: ~14 × 15 mm
- Push-push 메커니즘 (딸깍 꽂고 뽑기)
- 카드 detect 핀 (CD)
- 쓰기 보호 핀 없음

## 왜 선택했나

- push-push → 손쉬운 카드 교체 (wearable 환경)
- SDMMC 4-bit 지원 (SPI 보다 빠름)
- detect 핀 → MCU EXTI 로 카드 삽입 이벤트 감지

## 인터페이스 (SDMMC 4-bit)

- CMD
- CLK
- D0~D3
- Card Detect

## 외부 회로

- CMD·D0~D3 에 50kΩ 풀업 (카드 측 요구)
- 3.3V 바이패스 100nF + 10µF
- (옵션) ESD 다이오드 어레이

## 연결되는 블록

- [[Storage]]

## 데이터시트

- [ ] Hirose DM3AT 데이터시트 PDF
- [ ] ST AN5200 SDMMC 가이드 PDF
