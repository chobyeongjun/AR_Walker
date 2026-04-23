---
aliases: [HW Sync, Jetson Sync]
tags: [type/concept, domain/comm, status/decided]
type: concept
status: decided
---

# Hardware Sync for Jetson

> ✅ **사용자 확정** (2026-04-20, v3.10). 이전 제거 (v3.9) 철회.
> 근거: 사용자 "SYNC 추가야".

## 문제

Jetson 이 카메라 프레임에서 pose 를 추정해 MCU 에 보낼 때, **그 데이터가 언제 시점의 것인지** MCU 가 정확히 모름.

```
T1: 카메라 셔터 열림 (Jetson clock)
T2: pose 추정 완료 (T1 + ~10 ms, GPU jitter)
T3: UART 전송 시작 (T2 + ~2 ms)
T4: MCU 도착
T5: MCU 제어 루프 사용 시점
```

MCU 가 T5 에 데이터 받아 모터 명령하지만, 데이터는 사실 T1 시점 사용자 자세. `T5 - T1` 모르면 제어 불안정.

## 해법

MCU-Jetson 공유 하드웨어 이벤트 (GPIO 펄스) → 두 시스템 시계 정합.

### 방향 A: MCU → Jetson

MCU 가 1 kHz / 100 Hz 펄스 출력 → Jetson 이 GPIO 로 받아 자기 시계에 매핑 (가장 흔함).

### 방향 B: Jetson → MCU

Jetson (또는 카메라) 트리거 펄스 → MCU EXTI 가 "이 시점 카메라 셔터 열림" 기록.

**양방향 GPIO 핀 2개 + GND** = **JST-GH 3-pin** 커넥터.

## 비유

두 사람이 다른 손목시계 쓰면서 "3시에 만나자" 하면 못 맞음. 동시에 손뼉치면(SYNC) 그 순간을 기준으로 시계 보정 가능.

## 우리 보드에서의 적용

- [[STM32H743VIT6]] GPIO 2개 (SYNC_OUT, SYNC_IN) 🚩 CubeMX 확정 필요
- [[Comms]] 블록 내 JST-GH 3-pin (SYNC_OUT, SYNC_IN, GND)
- Jetson 측 GPIO 헤더 (Orin NX)
- 펌웨어: TIM DMA 로 pulse 출력, EXTI 로 pulse 타임스탬프 기록

## 비용

GPIO 핀 2개 + 3-pin 커넥터 = **~30 mm²** · 부품 0.5달러 수준.

## 없으면?

소프트웨어 NTP 같은 시간 보정 — 복잡, jitter 큼. HW SYNC 가 더 확실.

## 관련

- [[Comms]]
- [[Goals & Requirements]]
- [[Decisions Log]] — v3.9 제거, v3.10 재추가 기록
