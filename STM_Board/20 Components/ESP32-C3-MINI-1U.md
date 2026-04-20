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

> **✅ 사용자 확정 (2026-04-20)**: BLE 필요 — 실시간 원격 제어·모니터링·OTA 목적.
> 출처: 사용자 요청 (세션 대화). Legacy 에는 없음.

## 한 줄 요약

BLE 5.0 + WiFi 2.4GHz 통합 모듈, **IPEX 외부 안테나** — wearable 환경에서 신호 안정.

## ⚠️ BLE 역할 명확화 (중요)

BLE 5.0 스펙 (Bluetooth Core Spec 5.0):
- Connection interval 최소 **7.5 ms** (= 133 Hz)
- 실효 처리량 ~1 Mbps 이론 / 100~500 kbps 실전
- Latency 7.5ms + OS/app 지터

**→ BLE 는 500 Hz 저수준 제어 루프 불가.** 500Hz 는 MCU 내부 루프가 담당.

### BLE 의 우리 보드 용도

| 용도 | 주기 | BLE 가능? |
|---|---|---|
| 저수준 제어 (어드미턴스·전류) | 500 Hz | ❌ MCU 내부 |
| 고수준 명령 (보조 비율·모드 변경) | 1~10 Hz | ✅ |
| 텔레메트리 (SoC·로드셀·상태) | 10~100 Hz | ✅ |
| 파라미터 튜닝 (Kp, Kd 등) | 수동 | ✅ |
| OTA 펌웨어 업데이트 | 가끔 | ✅ |

## 왜 `-1U` (IPEX 외부 안테나) 인가

🚩 **variant 선택은 내 판단. 사용자 확정 필요 유지.**

참고: Espressif 데이터시트 ESP32-C3-MINI-1 vs ESP32-C3-MINI-1U 에서 PCB 내장 / IPEX 외부 두 variant 모두 공식 지원. 선택은 사용자 상황:

- `-1` (PCB 내장): 단일 부품, 15mm keep-out 영역 필요, 케이스/옷 가리면 신호 약화
- `-1U` (IPEX 외부): 안테나 케이스 밖 노출 가능, 추가 케이블 + 안테나 부품

wearable 에서 신호 안정성 원하면 IPEX 권장 (내 의견). 보드 단순화 원하면 PCB 내장.

## 사양 (공식 출처: Espressif ESP32-C3-MINI-1 데이터시트)

- BLE 5.0 + Wi-Fi 2.4 GHz 802.11b/g/n
- CPU: RISC-V single-core 160 MHz
- Flash: 4 MB 내장
- 전원: 3.3V, 피크 500 mA (TX)
- 패키지: SMD 모듈 13.2 × 16.6 mm

## 인터페이스 (MCU 와)

- UART (1 Mbps)
- EN 핀 (리셋 제어)
- IO9 핀 (BOOT 모드 — 부트로더 진입)

## 외부 회로

- 3.3V 라인 (500mA 피크 감당 → **22µF 탄탈 필수**)
- EN 풀업 10kΩ + 리셋 100nF
- IO9 풀업 10kΩ + BOOT 버튼 (IO9 → GND 모멘터리)
- IPEX 커넥터 → 외부 2.4GHz 안테나

## 연결되는 블록

- [[Wireless]]

## 데이터시트

- [ ] Espressif ESP32-C3-MINI-1 데이터시트 PDF
- [ ] Espressif "Hardware Design Guidelines" PDF
- [ ] "ESP32-C3 bring-up" 앱노트

## 관련

- [[Wireless]]
- [[Citations & Sources]] — BLE 스펙 출처 (Bluetooth Core 5.0)
