---
aliases: [Loadcell Amp Design, 로드셀 앰프]
tags: [type/concept, domain/sensor]
type: concept
---

# Loadcell Amplifier Design

## 문제

케이블 텐션 측정 → 1kHz force feedback 제어 루프에 사용.
- 로드셀 신호: 2~3 mV/V (매우 작음, 노이즈에 취약)
- 1 kHz 제어: ADC 최소 2 kSPS, 안정성 위해 10 kSPS+
- 두 케이블 **동시 샘플링** 필요 (phase error 방지)
- 전원 노이즈가 SNR 직격

## 해법 (HX711 → ADS131M04 업그레이드)

| 항목 | HX711 (이전) | [[ADS131M04]] (현재) |
|---|---|---|
| 해상도 | 24-bit | 24-bit |
| 샘플링 | **80 Hz** | **32 kSPS** |
| 채널 | 1 (멀티플렉서) | **4 동시** |
| 인터페이스 | bit-bang 2선 | SPI + DMA |
| SNR | 중 | **ENOB 20+** |
| PGA | 내장 (128×) | 내장 (1-128×) |

→ **1kHz 제어 + 성능 여유 + 추가 채널 2개**

## 핵심 설계 요소

### 1. 정밀 전압 레퍼런스

[[REF5025]] 2.5V, 3 ppm/°C, 7.5 µVpp 노이즈.
ADC FSR = ±VREF → VREF 안정 = force 측정 절대 정확도.

### 2. 아날로그 전원 분리

```
5V_buck ─[ferrite]─ TLV75533 (analog) ─► AVDD (ADS131M04, REF5025)
        ─[직결]───── TLV75533 (digital) ─► DVDD, MCU, 통신 IC
```

Ferrite bead (BLM18PG471, ~470Ω @ 100MHz) 로 디지털 스위칭 노이즈 차단.

### 3. 차동 안티앨리어싱 필터 (채널별)

```
S+ ──[R 1k]──┬──── AIN_P
             │
           [C 10nF] (차동)
             │
S- ──[R 1k]──┴──── AIN_N
             │
           [C 100pF] × 2 to GND (CM)
```

- 차단 주파수 (차동): 1/(2π·R·C) ≈ **16 kHz** (Nyquist 보호)
- CM 필터: 100 MHz 대역 제거

### 4. 로드셀 Excitation

- 옵션 A: 3.3V 디지털 → E+/E- (간단, 약간 노이즈)
- **옵션 B ⭐**: 별도 정밀 3.3V 또는 5V → 정확도 ↑

### 5. 커넥터 (채널당)

JST-GH 4-pin: `E+ / S+ / S- / E-`

## PCB 레이아웃 주의

- 아날로그 영역 ↔ 디지털 영역 **2mm 이상 간격**
- AGND / DGND **단일점 연결** (star ground)
- ADC 밑에 솔리드 GND 평면 ([[PCB Stackup 6-layer]] L2)
- 차동 라인 쌍으로 라우팅 (length matching)

## 관련

- [[ADS131M04]]
- [[REF5025]]
- [[TLV75533]]
- [[Loadcell Amp]] — schematic 블록 작업
- [[PCB Stackup 6-layer]]
