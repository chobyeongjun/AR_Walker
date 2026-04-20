---
aliases: [ZED X Mini, ZED Mini, stereo camera]
tags: [type/component, domain/sensor, phase/A, status/decided]
type: component
part_number: ZED X Mini
package: enclosed camera module
manufacturer: Stereolabs
voltage_range: PoDL (power over data link) 
interface: GMSL2
phase: A
status: decided
---

# ZED X Mini

## 한 줄 요약

Stereolabs 스테레오 카메라 (GMSL2) — 150g, IP67, 120 FPS. Jetson Orin NX 에 PCIe/ZED Link 로 연결. **MCU 에는 직접 연결 안 됨** (Jetson 전용).

## 사양

- 스테레오 카메라 (2× global shutter)
- 출력: 최대 2K @ 60fps / 720p @ 120fps
- 인터페이스: **GMSL2** (Jetson ZED Link 통해)
- 무게: 150g
- 방진방수: IP67
- 3D depth, 객체 추적, human pose estimation (ZED SDK)
- 전원: PoDL (Power over Data Link) — Jetson 측에서 공급

## 왜 선택했나

- Jetson 에 최적화 — ZED SDK 지원 (pose estimation, depth)
- IP67 → **wearable 외부 노출 OK**
- 150g → 무게 부담 적음
- 120 FPS → 빠른 사용자 동작 캡처
- 3D → 발 위치·관절 각도 3D 추정 가능

## 연결 (MCU 와 무관)

```
ZED X Mini ── GMSL2 케이블 ── Jetson Orin NX (ZED Link PCIe)
                                    │
                                    │ pose 추정 (ZED SDK + CUDA)
                                    ▼
                             Jetson Python 처리
                                    │
                                    │ UART + SYNC
                                    ▼
                            STM32H743 (제어)
```

우리 보드는 **Jetson 과 UART + SYNC 로만 통신** ([[Comms]]). ZED X Mini 자체는 보드 부품 아님.

## 보드에 영향

- **없음** (Jetson 전용)
- 단, Jetson UART 로 받는 pose 데이터가 ZED SDK 산출물 → 프레임 타임스탬프 → [[Hardware Sync for Jetson|HW SYNC]] 필요성의 근거

## 관련

- [[Jetson Orin NX]]
- [[Hardware Sync for Jetson]]

## 데이터시트

- [ ] Stereolabs ZED X Mini 스펙 시트
- [ ] ZED SDK 문서 (pose estimation API)
