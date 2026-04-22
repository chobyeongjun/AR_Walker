---
aliases: [Jetson Orin NX, Orin NX, Jetson]
tags: [type/component, domain/mcu, domain/comm, phase/A, status/decided]
type: component
part_number: Jetson Orin NX 16GB (900-13767-0000-000)
package: SoM (NVIDIA) + carrier board
manufacturer: NVIDIA
voltage_range: "9-20V DC in (carrier board, typical)"
interface: "MIPI CSI (via ZED Link card), UART (to MCU), GPIO (SYNC)"
phase: A
status: decided
---

# Jetson Orin NX

> **✅ Legacy HANDOFF 확정 + 사용자 동의.** TOPS 수치는 NVIDIA 공식 ("Sparse INT8" 기준 157, "Dense INT8" 기준 ~100).
> ⚠️ **로컬 PDF (`Jetson_Orin_NX.pdf`, `Jetson_Orin_NX_DesignGuide.pdf`) 는 14 KB = 에러 페이지.** 실제 내용 아님. 수동 재다운로드 필요 — §데이터시트 참고.

## 한 줄 요약

AI 컴퓨팅 SoM (100~157 TOPS). [[ZED X Mini]] 카메라로 body tracking → pose 출력 → [[STM32H743VIT6]] MCU 에 UART + SYNC 로 전송.

## 사양 (NVIDIA 공식 + ZED 데이터시트 교차 확인)

| 항목 | 값 |
|---|---|
| AI 성능 | **100 TOPS (Dense INT8)** / 157 TOPS (Sparse INT8 peak) |
| GPU | Ampere, 1024 CUDA cores, 32 Tensor cores |
| CPU | 8-core Arm Cortex-A78AE |
| RAM | **16 GB** LPDDR5 (128-bit, 102 GB/s bw) |
| PCIe | Gen4 x4 (Carrier board 따라) |
| USB | USB 3.2 Gen2 ×3, USB OTG micro ×1 |
| Ethernet | Gigabit |
| Display | HDMI |
| **MIPI CSI** | 4~6 lanes (ZED Link Duo 캡처 카드 경유) |
| UART | 여러 개 (MCU 통신용) |
| GPIO | 40-pin 호환 header (Jetson Nano 스타일) |
| 전원 | Carrier board 마다 상이, 일반적 9~20V DC |
| OS | Jetson Linux (L4T) / JetPack — **v35.1 이상** (ZED SDK 요구) |
| SKU | 900-13767-0000-000 (16GB 모듈) |

## 역할

- [[ZED X Mini]] 에서 프레임 캡처
- ZED SDK 로 pose 추정 (stereo depth, body tracking 18~38 keypoints)
- 고수준 제어 결정 (보행 위상, 보조 비율)
- MCU 로 pose + 명령 다운링크 (UART)
- 데이터 로깅·분석
- (옵션) OTA / WiFi 통신

## 보드(MCU)와의 인터페이스

→ [[Comms]] 블록

```
Jetson Orin NX                            STM32H743
       │                                        │
       │ UART (1 Mbps, DMA)                    │
       │ ──────────────────────────────────►  │ (pose, 제어 명령)
       │ ◄──────────────────────────────────  │ (상태, 텔레메트리)
       │                                        │
       │ GPIO SYNC_OUT → SYNC_IN               │
       │ ◄─────────────────────────────────►  │ (HW 시계 정합)
       │                                        │
       │ (선택) USB FS backup                  │
```

- **1 Mbps UART + DMA** (CRC 포함 프레임)
- **HW SYNC GPIO** ([[Hardware Sync for Jetson]])

## ZED X Mini 연결 (중요)

**Orin NX 는 GMSL2 직결 불가** — **ZED Link Duo 캡처 카드** 필요:

```
ZED X Mini ── FAKRA Z GMSL2 케이블 ── ZED Link Duo (MAX9296A deserializer)
                                                 │ MIPI CSI
                                                 ↓
                                          Jetson Orin NX
```

- ZED Link Duo 필요 외부 **12~19V 전원** (NX 사용 시)
- Samtec 포트 (AGX 전용) 직결은 NX 에서 불가

## 왜 Orin NX 16GB (legacy 확정)

1. **ZED X Mini + ZED Link Duo 지원** (데이터시트 System Requirements)
2. **16GB RAM** — ZED SDK + Body Tracking + 사용자 앱 여유
3. **100+ TOPS** — human pose 실시간 (30fps+, 18~38 keypoints)
4. AGX 대비 저전력·저발열 (wearable 패키지 적합)

## Carrier board 선택 (미확정)

Jetson Orin NX 는 SoM 이므로 carrier board 필요:

- NVIDIA 공식 Dev Kit (개발용, 크고 포트 많음)
- Seeed reComputer J401
- Connect Tech Rudi-NX
- Auvidea JNX30
- (Stereolabs) ZED Box Orin NX — ZED Link 통합, 우리 사용 시나리오에 적합

**사용자 확정 필요** — exosuit 폼팩터·전원·무게 기준으로.

## 데이터시트 (⚠️ 수동 다운로드 필요)

현재 레포의 PDF 는 14 KB = NVIDIA Developer 로그인 벽으로 인한 에러 페이지.

**정상 다운로드 경로:**
- Mouser mirror: https://www.mouser.com/datasheet/2/744/Seeed_Studio_06132023_102110781-3216398.pdf (브라우저)
- Design Guide (Mouser): https://www.mouser.com/pdfDocs/Jetson_Orin_NX_Series_and_Orin_Nano_Series_Design_Guide_DG-10931-001_v11.pdf
- NVIDIA Developer (로그인 필요): https://developer.nvidia.com/embedded/downloads

**다운로드 후 `17_System_Modules/` 에 교체.** 파일 크기 수백 KB ~ 수 MB 면 정품.

- [ ] Jetson Orin NX Datasheet (DS-10712)
- [ ] Jetson Orin NX Design Guide (DG-10931)
- [ ] JetPack / L4T 버전 호환성
- [ ] ZED SDK 지원 매트릭스

## 관련

- [[ZED X Mini]]
- [[Hardware Sync for Jetson]]
- [[Comms]]
- [[Citations & Sources]]

## 변경 이력

- v3.5 (2026-04-20): legacy HANDOFF 에서 "Orin NX 16GB, 157 TOPS" 채택
- **v3.14 (2026-04-22): ZED 데이터시트 교차검증** + TOPS 해석 (100 dense vs 157 sparse), **ZED Link Duo 캡처 카드 필요** 명시, carrier board 미확정 플래그, 로컬 PDF 무효 (수동 재다운로드 필요)
