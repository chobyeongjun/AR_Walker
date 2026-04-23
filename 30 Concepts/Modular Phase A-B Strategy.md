---
aliases: [Modular Strategy, Phase A-B Strategy]
tags: [type/concept, domain/process]
type: concept
---

# Modular Phase A-B Strategy

## 문제

모터 모델·전압·전류가 아직 미정 → 보드 설계 멈춰야 하나?

## 해법: Phase A / Phase B 분리

### 핵심 관찰

로직부 (MCU, 센서, 통신, 앰프) 는 **항상 5V/3.3V** 로 동작.
모터가 12V 든 48V 든 무관.
→ 로직 schematic 은 지금 freeze 가능.

### Phase A — 모터 무관 (지금 freeze)

모든 블록이 여기 속함. [[Modular Strategy]] 프로젝트 노트 참조.

- [[MCU Core]]
- [[Loadcell Amp]] ⭐
- [[Encoder]]
- [[Comms]]
- [[Wireless]]
- [[Storage]]
- [[USB Debug]]
- [[Safety UI]]
- [[Battery Monitor]]
- [[Logic Power]] (buck 토폴로지 포함)

### Phase B — 모터 결정 후 (보류)

- [[Motor Power]] 블록만 해당
- 영향 부품: 커넥터, eFuse MOSFET, 벌크 캡 V 정격, TVS, choke, 퓨즈, 폴리곤 너비

자세한 매트릭스: [[Voltage-Current Scaling]]

## 진행 방식

```
[지금]                              [모터 확정 후]
Phase A schematic              ──►  Phase B schematic 추가
(KiCad hierarchical sheets)         (motor_power.kicad_sch)
```

## 장점

- 로직 검증이 모터 도착 전에 완료
- 모터 변경 시 변경 범위 명확
- 부품 미리 발주·테스트 가능
- 로직판은 USB 5V 로 단독 동작 테스트 가능

## 단점

- 통합 PCB 레이아웃이 모든 블록 끝난 후에 진행됨
- 전원 인터페이스 정의 필요 (Phase A 에서 "5V 입력 받는다" 가정)

## PCB 수준 선택지 (P6 에서 결정)

- **단일 보드** — hierarchical sheet 로 설계, 발주 1회
- **2-PCB 분리** (로직판 + 파워판) — 보드-to-보드 커넥터 필요, 로직 독립 테스트 유리

## 관련

- [[Phase Plan]]
- [[Modular Strategy]]
- [[Voltage-Current Scaling]]
- [[Open Questions]]
