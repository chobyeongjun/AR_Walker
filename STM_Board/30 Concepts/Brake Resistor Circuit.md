---
aliases: [Brake Resistor, Braking Chopper, 제동 저항]
tags: [type/concept, domain/power, domain/safety]
type: concept
---

# Brake Resistor Circuit

## 문제

[[Regen Energy Protection|재생 에너지]] 가 1J 씩 지속·반복해서 들어오면:
- 벌크 캡만으론 한계 (V_peak 가 점점 상승 누적)
- TVS 는 반복 소산에 부적합 (수 J 소산 시 열폭주)
- **에너지를 계속 소산할 무언가** 필요

## 해법

버스 전압이 임계값 (26.5V) 초과 시, **MOSFET 을 켜서 저항으로 에너지 흘림**.

```
Vbus  ─────┬─────────────┬────────
           │             │
        비교기          [제동 저항]
        TLV3201          3Ω/50W
        26.5V 임계        │
           │             │
           ├──► GATE ──► MOSFET (IPB025N10N3)
           │             │
           │             ▼
           │           GND
           └── 저항 분압으로 26.5V 감지
```

## 설계 계산

### 임계 전압 26.5V 근거

| 값 | 설명 |
|---|---|
| 배터리 완충 | 25.2V |
| 마진 | +1.3V |
| **임계** | **26.5V** |
| V_peak (bulk+brake) | < 28V (안전) |
| AK 모터 내압 | 40V |

### 저항값 3Ω 계산

```
V = 28V (최대), R = 3Ω
피크 전류 = 28/3 = 9.33 A
피크 전력 = 28 × 9.33 = 261 W
평균 듀티 ≈ 20% (사이클 기준)
평균 전력 = 261 × 0.2 = 52 W → 50W 저항 OK
```

### 부품

| 부품 | 사양 | 파트넘버 | 수량 |
|---|---|---|---|
| 비교기 | 단전원, 저전력 | TI **TLV3201DCKR** | 1 |
| 저항 (분압 상단) | 100kΩ 0.1% | - | 1 |
| 저항 (분압 하단) | 10kΩ 0.1% | - | 1 |
| 제동 MOSFET | 100V/130A | Infineon **IPB025N10N3 G** | 1 |
| 제동 저항 | 3Ω/50W | Arcol HS50 3R0 J | 1 |

### 비교기 분압 (26.5V 임계)

```
26.5V × (10k / (100k + 10k)) = 2.41V → 비교기 기준 (Vref)
```

## 동작 원리

1. 정상 운행: V_bus = 24~25.2V → 비교기 출력 LOW → MOSFET OFF
2. 회생 발생: V_bus 상승
3. V_bus ≥ 26.5V: 비교기 HIGH → MOSFET ON → 저항으로 전류 흐름
4. V_bus 하락 → 임계 아래 → MOSFET OFF
5. 반복 (chopping)

## 실패 모드 보호

- 비교기 TLV3201 자체 전원: 5V 또는 3.3V (분압 전압은 공통모드 범위 이내)
- MOSFET 스위칭 노이즈 → 커패시터 가까이 배치, 게이트 저항 10Ω 삽입
- 저항 열 소산 → **외부 방열판 필수** (50W 소산)
- MOSFET TO-263 패드 → 2oz 동박 방열 패드 최소 4cm²

## PCB 레이아웃

```
MOSFET IPB025N10N3 → PCB 모서리 배치 또는 외부 방열판 연결
저항 HS50 3Ω → PCB 외부 별도 방열판 (발열 큼)
```

## 관련

- [[Regen Energy Protection]]
- [[Motor Power Isolation]]
- [[Motor Power]]
- [[_legacy/EXOSUIT_PROTECTION]]
