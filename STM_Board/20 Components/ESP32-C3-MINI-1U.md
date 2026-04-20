---
aliases: [ESP32-C3, ESP32-C3-MINI-1U, BLE 모듈]
tags: [type/component, domain/comm, phase/A, status/decided]
type: component
part_number: ESP32-C3-MINI-1U
package: SMD module (13.2 x 16.6 mm)
manufacturer: Espressif
voltage_range: 3.3V
interface: UART (to MCU) + IPEX antenna
phase: A
status: decided
---

# ESP32-C3-MINI-1U

## 한 줄 요약

BLE 5.0 + WiFi 2.4GHz 통합 모듈, **IPEX 외부 안테나** — wearable 환경에서 신호 안정.

## 왜 `-1U` (IPEX) 인가

- [[Wireless]] 블록에서 PCB 내장 (`-1`) vs IPEX 외부 (`-1U`) 비교
- 착용 시 보드가 케이스·옷에 가려짐 → PCB 안테나 신호 약화
- IPEX 케이블로 안테나만 외부 노출 → 연결 안정성 ↑
- 추가 부품: IPEX MHF1 케이블 + 2.4GHz dipole 안테나

## 인터페이스

- MCU 와 UART (1 Mbps)
- EN 핀 (MCU 제어 리셋)
- IO9 핀 (BOOT 모드)
- GPIO 2-3개 (디버그)

## 역할

- BLE 5.0 — 폰 앱으로 실시간 [[Battery SoC Estimation|SoC]] · 제어 상태 모니터링
- WiFi — OTA 펌웨어 업데이트, 로그 업로드 (옵션)
- STM32 측에서는 UART 만 보면 됨 (ESP-IDF 펌웨어 별도)

## 전원

- 3.3V, 500mA 피크 (TX 시)
- 22µF tantalum 필수 (dip 방지)

## 연결되는 블록

- [[Wireless]]

## 데이터시트

- [ ] Espressif ESP32-C3-MINI-1 데이터시트 PDF
- [ ] Espressif "Hardware Design Guidelines" PDF
- [ ] "ESP32-C3 bring-up" 앱노트
