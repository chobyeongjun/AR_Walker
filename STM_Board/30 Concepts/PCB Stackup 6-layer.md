---
aliases: [6L Stackup, PCB Stackup]
tags: [type/concept, domain/pcb]
type: concept
---

# PCB Stackup 6-layer

## 문제

4-layer 60×60mm 와 6-layer 50×50mm, 어느 쪽이 나은가?
사용자 요청: **layer ↑ 로 사이즈 ↓ + 성능·안전.**

## 해법: 6-layer 양면 실장

### 스택업

```
Layer 1 (TOP)     — 신호 + 주요 부품 (MCU, ESP32, microSD, 커넥터)   2 oz
Layer 2 (GND1)    — 솔리드 GND 평면 (return path, EMI shield)         0.5 oz
Layer 3 (SIG/PWR) — 모터 전원 폴리곤 + 디지털 신호 라우팅              1 oz
Layer 4 (PWR)     — 3.3V/5V 평면                                       1 oz
Layer 5 (GND2)    — 솔리드 GND 평면 (analog return)                    0.5 oz
Layer 6 (BOT)     — 신호 + 패시브 (디커플링 캡, 풀업, 작은 IC)         2 oz
```

**핵심:** 모든 신호층이 GND 평면에 **인접** → impedance 제어 + EMI 급감.

### 제조 사양

| 항목 | 값 |
|---|---|
| 보드 두께 | 1.6 mm (1.0 mm 옵션) |
| 최소 트레이스/간격 | 4 mil / 4 mil |
| 최소 비아 | 0.2 mm 드릴 / 0.45 mm 패드 |
| Via-in-pad | 사용 (epoxy fill + cap plating) |
| 외층 동박 | **2 oz** (30A 모터 트레이스) |
| 내층 동박 | 1 oz |
| 표면 처리 | **ENIG** (산화 방지 + SMT 정확도) |

## 4L vs 6L vs 8L 이득 비교

| 항목 | 4L | 6L | 8L |
|---|---|---|---|
| 보드 사이즈 | 60×60 | **50×50** | 45×45 |
| 신호-GND 인접 | 일부만 | 전부 | 전부 |
| Crosstalk | 보통 | **-10 dB** | 매우 낮음 |
| [[ADS131M04]] SNR | 기준 | **+6~10 dB** | +10 dB |
| USB HS eye 마진 | 빠듯 | **여유** | 매우 여유 |
| 모터 전원 임피던스 | 1× | 0.5× | 0.25× |
| EMI | FCC 위태 | **통과** | 통과 |
| PCB 비용 (5장) | $15/장 | $45/장 | $70/장 |
| 총 5장 추가 비용 | 기준 | +$150 | +$275 |

## 성능·안전 이득의 근거

1. **GND 평면 2개** → motor GND ↔ logic GND 단일점 연결, 양쪽 임피던스 낮음
2. **전원 평면** → MCU 디커플링 캡 효과 극대화 (평면 자체가 캡 역할)
3. **신호-GND 인접** → 반사·crosstalk 차단, USB HS 480Mbps 안정
4. **라우팅 자유도** → 컴팩트하면서 고속 신호 라우팅 가능

## 8-layer 옵션

극한 컴팩트 (45×45mm) 원하면:

```
L1 TOP    — 신호 + 부품
L2 GND    — 솔리드 GND
L3 SIG    — 디지털 라우팅
L4 PWR    — 모터 전원 폴리곤 1
L5 PWR    — 모터 전원 폴리곤 2 (병렬, 30A 분산)
L6 SIG    — 아날로그 (loadcell, current sense)
L7 GND    — 솔리드 GND (analog return)
L8 BOT    — 신호 + 패시브
```

→ 모터 전원 dual plane 으로 발열 분산 best.

## 결론

**6L 50×50mm** 채택 — 성능·사이즈·비용의 sweet spot.
8L 은 5mm 더 줄이고 싶을 때 P6 단계에서 재검토.

## 관련

- [[Size Budget]]
- [[Motor Power Isolation]]
- [[Loadcell Amplifier Design]]
