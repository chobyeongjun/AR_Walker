---
aliases: [ZED X Mini, ZED Mini, stereo camera, stereolabs]
tags: [type/component, domain/sensor, phase/A, status/decided]
type: component
part_number: "ZED X Mini (SKU ZED-311210 / 311220)"
package: enclosed stereo camera (aluminum body)
manufacturer: Stereolabs
voltage_range: PoC via GMSL2 (Jetson 측 공급)
interface: GMSL2 (FAKRA Z connector) → Jetson
phase: A
status: decided
---

# ZED X Mini

> **✅ 사용자 확정 + 데이터시트 v1.2 직접 읽어 검증 완료**
> 출처: `50 References/Datasheets/17_System_Modules/ZED_X_Datasheet_v1.2.pdf` (Stereolabs 공식, v1.2)

## 한 줄 요약

IP67 스테레오 카메라 (93.6×31.8×36.7mm, 151g). 내장 IMU 포함, GMSL2 로 **Jetson Orin NX + ZED Link Duo 카드** 에 연결. wearable 가능 경량 폼팩터.

## 기본 사양 (데이터시트 §ZED X General Specifications)

| 항목 | 값 |
|---|---|
| **치수** | **93.6 × 31.8 × 36.7 mm** |
| **무게** | **151 g** |
| **베이스라인** (스테레오 간격) | 5 cm |
| **방진·방수** | **IP67** (outdoor / agricultural / medical 호환) |
| 동작 온도 | -20°C ~ +55°C |
| 인터페이스 | **GMSL2** (FAKRA Z 커넥터, lockable) |
| 전원 | **PoC — Power over Coax** (GMSL2 케이블 통해 Jetson/캡처 카드 측 공급) |
| 케이블 길이 | 0.3m / 1.5m / 5m / 10m (FAKRA Z 표준) |
| 마운팅 | 1× 1/4"-20 UNC (삼각대 호환) · 2× M3×0.5 (바닥) · 4× M4×0.7 (뒷면) |
| 보증 | 2년 하드웨어 보증 |

## 이미지 센서 (§Sensor Stack Specifications)

| 항목 | 값 |
|---|---|
| 센서 | 1/2.6" 2.3 MP RGB × 2 (스테레오) |
| 픽셀 배열 | 1928 × 1208 |
| 픽셀 크기 | 3 μm × 3 μm |
| 셔터 | 전자 **Global Shutter** 동기 (카메라 간 frame-level sync **< 100 μs**) |
| 출력 해상도 (side-by-side) | 2× 1920×1200 @ 15/30/60 fps · 2× 1920×1080 @ 15/30/60 fps · **2× 960×600 @ 120 fps** |
| 출력 포맷 | RAW10 |
| Max S/N | 38 dB |
| Dynamic range | 71.4 dB |
| Sensitivity | 22.3 Ke/Lux·s |

## 내장 IMU (보조 센서)

> ⚠️ **주의:** ZED 에 IMU 가 내장돼 있지만, 이는 **카메라 자체의 pose/motion** 용 (Visual-Inertial SLAM). 사용자 몸 자세는 별도 [[EBIMU24GV6]] 를 사용.

| 센서 | 범위 | 해상도 | 노이즈 | ODR |
|---|---|---|---|---|
| Accelerometer (16-bit) | ±12 g | 0.36 mg | 3.2 mg | **400 Hz** |
| Gyroscope (16-bit) | ±1000 dps | 0.03 dps | 0.10 dps | **400 Hz** |
| 감도 오차 | ±0.5% | | | |

## 렌즈 옵션 (사용자 확정 필요)

데이터시트에 **2.2mm (광각) / 3.8mm (표준)** 두 옵션 있음. SKU 다름:

| 렌즈 | FOV (H×V×D) | Aperture | Depth Range | Body Tracking | 적합 용도 | SKU |
|---|---|---|---|---|---|---|
| **2.2mm** ⭐ | 110° × 80° × 120° | f/2.2 | 0.1 ~ 8 m | up to 6 m | 실내 근거리, 사용자 근접 | ZED-311210 |
| 3.8mm | 80° × 52° × 91° | f/1.8 | 0.15 ~ 12 m | up to 6 m | 실외, 장거리 | ZED-312210 |

**Exosuit 에 권장: 2.2mm** — wearable 에선 사용자 몸 가까이 카메라 배치, 광각이 body tracking 유리. 단, 사용자 결정 필요 (내 추천).

편광 필터 옵션 (polarizer): `ZED-311220` (2.2mm + polarizer) 등 — 야외 반사 감소, 실내면 불필요.

## 시스템 호환 (§System Requirements)

**지원 Jetson:** NVIDIA Jetson AGX Orin / AGX Xavier / **Orin NX** / Xavier NX ✓
**OS:** Jetson Linux (L4T) **v35.1 이상** (JetPack 5.0+)

### ⚠️ **Orin NX 는 직결 불가 — ZED Link 캡처 카드 필요**

우리 Jetson 은 **Orin NX** 이므로 GMSL2 직접 포트 없음. 다음 중 택1:

| 캡처 카드 | 지원 | Deserializer | 전원 | 사용 케이스 |
|---|---|---|---|---|
| **ZED Link Duo** ⭐ | Xavier/Orin (NX/Nano 포함) | MAX9296A | **외부 12~19V 필요** (NX 사용 시) | 카메라 1~2대, 우리 사용 |
| ZED Link Quad | Xavier/Orin | 2× MAX96712 | 12~19V 외부 | 카메라 4대 (과잉) |

→ **ZED Link Duo + 12V 외부 전원** 이 표준 구성.

## 연결 토폴로지

```
ZED X Mini (IP67, 151g)
     │
 FAKRA Z GMSL2 케이블 (예: 1.5m, CBL-320200)
     │
 ZED Link Duo 캡처 카드 (MAX9296A, 12~19V 외부 전원)
     │
 MIPI CSI / Samtec 포트
     │
 NVIDIA Jetson Orin NX (ZED SDK)
     │
 UART (1 Mbps, DMA) + SYNC GPIO
     │
 STM32H743 (제어)
```

## SDK 기능 (§ZED X SDK)

| 기능 | 용도 (exosuit 관련) |
|---|---|
| Stereo Capture | RGB + depth 프레임 |
| Depth Sensing | 거리 맵, 3D 포인트 클라우드 |
| **Positional Tracking (6DoF)** | 카메라 pose, WVGA 최대 **100 Hz** |
| Spatial Mapping | 실시간 3D 맵 |
| **Body Tracking** | 스켈레톤 (18/34/38 keypoints) 2D + 3D + 회전 — exosuit 보조 제어에 핵심 |
| Object Detection | 일반 객체 2D+3D |

## Exosuit 활용 시사점

1. **사용자 자세 추정**: Body Tracking 이 18~38 keypoints 제공 → 관절 각도 계산 → MCU 로 pose 전송
2. **Pose 출력 속도**: WVGA 최대 100 Hz → [[Hardware Sync for Jetson|HW SYNC GPIO]] 와 결합 시 1ms 이내 타임스탬프 정합 가능
3. **SYNC 정합도 향상**: ZED 자체 multi-camera sync 가 100 μs 수준 → MCU sync pulse 와 연결하면 sub-ms 정합도 실현 가능
4. **IMU 이중화**: ZED 내장 IMU (카메라 프레임 pose) + [[EBIMU24GV6]] (사용자 몸) = Visual-Inertial 보행 추정 정확도 ↑

## 보드 영향

- **없음** (ZED 는 Jetson 전용, MCU 와 직결 X)
- 단, ZED Link Duo 캡처 카드 12V 외부 전원 필요 → 배터리 시스템 설계 시 반영 (Phase B)
- Jetson 과 MCU 사이 UART + SYNC 로만 통신

## 관련

- [[Jetson Orin NX]] — 호스트
- [[Hardware Sync for Jetson]] — pose 타임스탬프 정합
- [[EBIMU24GV6]] — 사용자 몸 IMU (역할 다름)

## 데이터시트 (저장 완료)

- [x] `ZED_X_Datasheet_v1.2.pdf` (10.5 MB, v1.2)
- [ ] ZED SDK 문서 (body tracking API) — Stereolabs 온라인 문서
- [ ] ZED Link Duo 캡처 카드 스펙 — 별도 문서 확보 필요

## 변경 이력

- v3.5 (2026-04-20): legacy HANDOFF 에서 "ZED X Mini" 채택 (스펙 추정)
- **v3.14 (2026-04-22): 데이터시트 직접 읽어 전 사양 검증**
  - 치수 93.6×31.8×36.7mm 확정
  - 무게 151g 확인 (legacy "150g" 근사 정확)
  - 내장 IMU 존재 명시 (Accel ±12g, Gyro ±1000dps, 400Hz)
  - **Orin NX 는 ZED Link Duo 캡처 카드 필요** (중요 신규 요건)
  - 렌즈 옵션 2.2mm / 3.8mm 두 가지 (사용자 선택 필요)
  - 외부 12~19V 전원 필요 (캡처 카드)
  - Body tracking 18~38 keypoints 확인 → exosuit 관절 각도 추정 가능

## 관련

- [[Comms]]
- [[Citations & Sources]]
- [[_legacy/HANDOFF]]
