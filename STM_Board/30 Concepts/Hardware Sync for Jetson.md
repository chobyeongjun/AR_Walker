---
aliases: [HW Sync, Jetson Sync]
tags: [type/concept, domain/comm, status/removed]
type: concept
status: removed
---

# Hardware Sync for Jetson (REMOVED)

> ❌ **v3.9 에서 제거됨.** 사용자 확정: 제어 아키텍처상 필요 여부 불명 → "모르면 제거" 원칙 적용.
> 나중에 Jetson pose 를 MCU 제어 루프 입력으로 쓰게 되면 재활성화 가능.

## 제거 이유

- Legacy 문서 (`HANDOFF.md`) 에 SYNC 언급 없음 — 내가 추가한 개념
- 사용자: "이게 제대로 작동할 지 + 왜 필요한 지 잘 모르겠다"
- 우리 시스템 제어 아키텍처 2 옵션:
  - (a) Jetson pose → MCU 제어 루프 입력 → SYNC 필요
  - (b) MCU 자체 센서 (로드셀·인코더·EBIMU) 로만 제어, Jetson = 고수준 명령만 → SYNC 불필요
- 현재는 (b) 가정. 필요 시 추가 가능 (GPIO 2개 + 3-pin 헤더 공간은 항상 여유).

## 아래 내용은 참고용 보존 (재활성화 시 사용)

### 원래 개념: 왜 필요할 수 있나

Jetson 이 카메라 프레임에서 pose 를 추정해 MCU 에 보낼 때, **그 데이터가 언제 시점의 것인지** MCU 가 정확히 모름.

```
T1: 카메라 셔터 열림 (Jetson clock)
T2: pose 추정 완료 (T1 + ~10 ms, GPU jitter)
T3: UART 전송 시작 (T2 + ~2 ms)
T4: MCU 도착
T5: MCU 제어 루프 사용 시점
```

MCU 가 T5 에 데이터 받아 모터 명령하지만, 데이터는 사실 T1 시점 사용자 자세. `T5 - T1` 모르면 제어 불안정.

### 해법

MCU-Jetson 공유 하드웨어 이벤트 (GPIO 펄스) → 두 시스템 시계 정합.

### 재활성화 조건

Jetson pose 를 MCU 제어 루프에 **실시간 입력** 으로 쓰게 되면 추가:
- GPIO 2개 (SYNC_OUT, SYNC_IN)
- JST-GH 3pin 커넥터
- 펌웨어: TIM DMA pulse 출력, EXTI 타임스탬프

## 관련

- [[Comms]] — SYNC 커넥터·핀 제거됨 (v3.9)
- [[Decisions Log]] — 제거 결정 기록
