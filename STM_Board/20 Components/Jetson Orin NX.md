---
aliases: [Jetson Orin NX, Orin NX, Jetson]
tags: [type/component, domain/mcu, domain/comm, phase/A, status/decided]
type: component
part_number: Jetson Orin NX 16GB
package: SoM (NVIDIA) + carrier board
manufacturer: NVIDIA
voltage_range: 19V DC in (carrier)
interface: "PCIe (ZED Link), UART (to MCU), GPIO (SYNC)"
phase: A
status: decided
---

# Jetson Orin NX

## 한 줄 요약

157 TOPS AI 컴퓨팅 모듈. [[ZED X Mini]] 스테레오 카메라로 pose 추정 후 [[STM32H743VIT6]] MCU 에 UART 전송. 보드와는 **시스템 레벨** 인터페이스.

## 사양

- AI: 157 TOPS (Sparse INT8)
- GPU: Ampere 1024 cores
- CPU: 8× Arm Cortex-A78AE
- RAM: **16 GB**
- PCIe Gen4 x8 (ZED Link 용)
- Ethernet, USB 3.2, UART, GPIO
- 전원: 19V DC in (carrier board)
- OS: Linux for Tegra (L4T)

## 역할

- ZED X Mini pose estimation (ZED SDK + CUDA)
- 고수준 제어 모드 전환 (보행 위상 판단, 보조 비율)
- 데이터 로깅·분석
- MCU 에 명령/pose 다운링크

## 보드와의 인터페이스

→ [[Comms]] 블록

```
Jetson Orin NX                  STM32H743
       │                            │
       │ UART (1 Mbps, DMA)       │
       │ ──────────────────────►   │ (pose 데이터, 제어 명령)
       │ ◄──────────────────────   │ (상태 텔레메트리)
       │                            │
       │ GPIO SYNC_OUT → SYNC_IN   │
       │ ──────────────────────►   │
       │                            │
       │ (옵션) USB FS backup      │
```

- **1Mbps UART + DMA** (CRC 포함 packet)
- **HW SYNC GPIO** — pose 프레임 타임스탬프 정합 ([[Hardware Sync for Jetson]])
- 위 두 개만 있으면 OK. Ethernet, USB HS 등 미사용.

## 왜 Orin NX 16GB (legacy 명시)

- **ZED X Mini GMSL2 → PCIe** : ZED Link 필요 → Orin NX 이상 요구
- 16GB RAM : ZED SDK + OS + 사용자 앱 여유
- 157 TOPS : human pose estimation 실시간 (30fps+)
- Nano (8GB) 로도 동작 가능하나 여유 ↓

## 관련

- [[ZED X Mini]]
- [[Hardware Sync for Jetson]]
- [[Comms]]

## 데이터시트

- [ ] NVIDIA Jetson Orin NX 데이터시트
- [ ] JetPack / L4T 버전 호환성
- [ ] ZED SDK 버전
