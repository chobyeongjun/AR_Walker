---
aliases: [Inrush Limiting, 인러시 제한]
tags: [type/concept, domain/power, domain/safety]
type: concept
---

# Inrush Current Limiting

## 문제

보드 처음 전원 인가 시, **벌크 캡 (11,280µF)** 이 비어있음 → 순간 충전 전류 폭주.

```
초기 순간:
  V_cap = 0V, V_bat = 24V, C = 11,280 µF
  
  트레이스 저항만 ≈ 50mΩ:
  I_inrush = V/R = 24/0.05 = 480 A    ← 커넥터·MOSFET·퓨즈 녹음
```

XT60 정격 30A 지속인데 순간 480A 흐르면:
- 접촉 아크 발생
- 배터리 BMS 과전류 트립
- MOSFET 게이트 손상 가능
- 반복 시 피로 파괴

## 해법 (2단계 NTC + bypass 릴레이)

```
           NTC 제한 경로                 bypass 경로
[V+ in]──[NTC 2Ω]──[NTC 2Ω]──┬──[릴레이 접점]──[V+ bus]
                              │                   │
                              └───────────────────┘
                              
릴레이 코일: [RC 지연 회로 또는 MCU GPIO 제어] → 5V
```

### 동작

1. **전원 인가 순간**: NTC 직렬 4Ω → 인러시 6A 로 제한
   ```
   I_inrush = 24V / 4Ω = 6 A (감당 가능)
   τ = R·C = 4 × 11,280µF = 45 ms (충전 시정수)
   ```
2. **NTC 자기 가열**: 전류로 NTC 온도↑ → 저항↓ → 효율↑
3. **충전 완료 (~100ms 후)**: 릴레이 코일 활성 → 접점 닫힘 → NTC bypass
4. **정상 운행**: 릴레이 접점으로 저저항 통로 → NTC 발열 없음

## 부품

| 부품 | 사양 | 파트넘버 | 수량 |
|---|---|---|---|
| NTC | 2Ω, 5A 정격, 2J 에너지 | Ametherm **SL08-2R005** | 2 (직렬) |
| Bypass 릴레이 | 5V DC, 10A 접점 | Omron **G5V-1-DC5** | 1 |
| 릴레이 구동 | N-MOSFET (로직) + 플라이백 다이오드 | 2N7002 + 1N4148 | 1조 |
| 릴레이 제어 RC | 10kΩ + 10µF (지연 ~100ms) | - | 1 |

### NTC 직렬 vs 단일

**왜 2개 직렬 (4Ω) 인가?**

```
단일 2Ω 반복 동작:
  자기 가열 누적 → 식을 시간 없이 저항 감소
  다음 인러시 때 제한 기능 감소 → 과전류 통과

직렬 2개 4Ω:
  각 NTC 에너지 분산 → 2.88 J / NTC (정격 5 J 이내)
  반복 가능
```

## 릴레이 제어 옵션

### 옵션 A: 자동 (간단, 권장)
- RC 회로로 100ms 지연 → 자동 ON
- MCU 개입 없음

### 옵션 B: MCU 제어 (고급)
- MCU 부팅 후 GPIO → 릴레이 구동
- 장점: 에러 시 MCU 가 OFF 할 수 있음
- 단점: MCU 부팅 전에는 NTC 만으로 보호

→ 우리 선택: **옵션 A** (단순함, 자기 완결)

## 부가 효과

- 재생 에너지 충격으로 인한 **전원 재인가** 같은 가상 상황에도 대비
- 배터리 교체 시 hot-plug 안전성 ↑
- 사용자 실수 (전원 빠른 ON/OFF) 보호

## 관련

- [[Regen Energy Protection]]
- [[Motor Power Isolation]]
- [[Motor Power]]
- [[_legacy/EXOSUIT_PROTECTION]]
