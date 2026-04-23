---
aliases: [Wireless Block]
tags: [type/block, phase/A, domain/comm]
type: schematic_block
phase: A
status: pending
kicad_sheet: wireless.kicad_sch
---

# Wireless

> **✅ 사용자 확정**: BLE 필요. 출처: 2026-04-20 세션 대화.

## 목적

[[ESP32-C3-MINI-1U]] (BLE 5.0 + WiFi) 를 코프로세서로 배치. MCU 는 UART 1개로만 접근.

## ⚠️ BLE 용도 (한계 명시)

- **저수준 500Hz 제어는 BLE 불가** (Bluetooth 5.0 스펙 최소 7.5ms interval)
- BLE 는 고수준 명령·텔레메트리·OTA 용 (→ [[ESP32-C3-MINI-1U]] 문서 "BLE 역할" 섹션 참고)

## 핵심 부품

- [[ESP32-C3-MINI-1U]] 모듈
- IPEX MHF1 케이블 + 2.4GHz 외부 안테나
- 3.3V 바이패스 22 µF tantalum + 100nF
- EN 풀업 10kΩ + 리셋 100nF
- IO9 풀업 10kΩ (BOOT 모드)
- BOOT 버튼 (IO9 → GND 모멘터리)

## 핀맵 (MCU 측)

🚩 **CubeMX `.ioc` 에서 충돌 검사 필요**

| 신호 | MCU 핀 (제안) | 비고 |
|---|---|---|
| UART3_TX | PD8 | 🚩 |
| UART3_RX | PD9 | 🚩 |
| ESP_EN | PE0 | 🚩 |
| ESP_IO9 | PE1 | 🚩 |

## 외부 회로

- 3.3V 전용 라인 (TX 피크 500mA 대비 22µF tantalum 필수 — Espressif HW Design Guidelines)
- IPEX 커넥터 → 모듈 안테나 포트 트레이스 **50Ω 임피던스 매칭**

## 관련 부품

- [[ESP32-C3-MINI-1U]]

## 데이터시트

- [ ] Espressif ESP32-C3-MINI-1 데이터시트
- [ ] Espressif HW Design Guidelines
- [ ] ESP32-C3 Bring-up 앱노트

## 작업 체크리스트

- [ ] 모듈 풋프린트 확인 (Espressif 제공)
- [ ] IPEX 커넥터 풋프린트
- [ ] 50Ω 안테나 트레이스
- [ ] UART 플로우 컨트롤 사용 여부 결정
- [ ] CubeMX 핀 충돌 검사
- [ ] sheet 그리기
