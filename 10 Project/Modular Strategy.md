---
aliases: [Modular Strategy]
tags: [project, strategy]
---

# Modular Strategy

[[Modular Phase A-B Strategy]] 개념을 프로젝트 진행에 적용한 문서.

## 핵심 관찰

로직부 (MCU, 센서, 통신, 앰프, BLE) 는 **항상 5V/3.3V** 로 동작 → 모터가 12V든 48V든 무관.
→ 모터 결정 전에 로직부 100% freeze 가능.

## Phase A — 모터 무관 (지금 freeze)

| 블록 | 부품 | 의존성 |
|---|---|---|
| [[MCU Core]] | [[STM32H743VIT6]] | ❌ |
| [[Loadcell Amp]] ⭐ | [[ADS131M04]] + [[REF5025]] | ❌ |
| [[Comms]] | [[ISO1050]] + [[MGJ2D05050SC]] + UART 헤더 | ❌ (CANopen 표준) |
| [[Wireless]] | [[ESP32-C3-MINI-1U]] | ❌ |
| [[Storage]] | [[DM3AT microSD]] | ❌ |
| [[USB Debug]] | USB-C + [[TPD4S012]] | ❌ |
| [[Safety UI]] | [[74LVC1G08]] + 버튼·LED | △ enable 신호 방향만 |
| [[Battery Monitor]] | [[INA228]] + 1mΩ shunt | △ shunt 위치만 |
| [[Logic Power]] | 5V→3V3 [[TLV75533]] × 2 | ❌ |
| Buck IC | [[TPS62933]] (60V 입력 커버) | △ 입력 캡 V 정격만 |

## Phase B — 모터 결정 후 (보류)

| 블록 | Voltage 의존 | Current 의존 |
|---|---|---|
| DC 입력 커넥터 | ❌ | ✅ |
| 모터 출력 커넥터 | ❌ | ✅ |
| eFuse 외부 MOSFET V_DS / RDS(on) | ✅ | ✅ |
| 벌크 캡 V 정격 | ✅ | ❌ |
| TVS 클램프 전압 | ✅ | ❌ |
| Common-mode choke | ❌ | ✅ |
| 백업 퓨즈 | ❌ | ✅ |
| 모터 폴리곤 너비 | ❌ | ✅ |

→ 구체 매트릭스는 [[Voltage-Current Scaling]] 참조

## 진행 방식

```
[지금]                              [모터 확정 후]
                                   
Phase A schematic 그리기      ──►  Phase B schematic 추가
(KiCad hierarchical sheet)         (motor_power.kicad_sch)
                                   
부품 발주·보관 가능                부품 사이즈 확정 후 발주
                                   
PCB 레이아웃은 B 끝난              통합 PCB 레이아웃
후 통합                            (또는 mezzanine 분리)
```

## 옵션: 단일 보드 vs 로직+파워 2-PCB

- **단일 보드** ⭐ — hierarchical sheet 로 설계, 발주 1회
- 2-PCB 분리 — 로직판 독립 테스트 가능, 보드-to-보드 커넥터 필요
- 결정: P6 PCB 레이아웃 단계에서 재검토

## 장점

- 로직 schematic 검증이 모터 도착 전에 완료
- 모터 변경 시 변경 범위가 명확 (Phase B 만)
- 부품 미리 발주·테스트 가능
- 단독 로직판 테스트 가능 (USB 5V 공급)
