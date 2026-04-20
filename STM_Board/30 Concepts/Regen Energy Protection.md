---
aliases: [Regen Protection, 재생 에너지 보호]
tags: [type/concept, domain/power, domain/safety]
type: concept
---

# Regen Energy Protection

## 문제 (이전 보드 실제 파손 사례)

케이블 드리븐 exosuit 는 **pull-release 반복** — 모터가 사람 동작을 보조하다가 순간 릴리즈하면 **케이블 탄성에너지 + 관성 에너지** 가 모터 coil 로 되돌아옴 (regenerative current).

```
케이블 탄성에너지 (모터당):
  E = F × x / 2 = 100N × 5mm / 2 = 0.25 J

4 모터 동시 릴리즈:
  E_total = 1 J

배터리 BMS 가 역전류 거부 → 에너지 갈 곳 없음:
  → 버스 캐패시터로 흘러감 → 전압 상승

C_bus = 100µF 만 있으면:
  V_peak = √(2E/C + V₀²) = √(2×1 / 100µF + 24²)
         = √(20,000 + 576)
         = 143 V    ← IC 절대최대 훌쩍 초과, 즉사

C_bus = 10,000 µF 이상:
  V_peak = √(2×1 / 10,000µF + 24²) = √(200 + 576) = 27.9 V  ← 안전
```

**이전 보드가 타 버린 주 원인.**

## 해법 (계층적 3중 방어)

### 1. 대용량 벌크 캐패시터 (충격 흡수)

```
필요 C_min 계산 (V_limit = 28V, 배터리 24V, E = 1J):
  ΔV² = V_limit² - V₀² = 784 - 576 = 208
  C_min = 2E/ΔV² = 2×1 / 208 = 9,615 µF

확정: 4,700µF × 2 = 9,400 µF (벌크) + 모터 로컬 470µF × 4 = 1,880 µF
     → 총 11,280 µF → V_peak 27.9V < 28V 임계 ✅
```

**부품**: Rubycon 35ZLH4700M (4700µF / 35V 전해, 2개 병렬) + Panasonic SVPD471M35 (OS-CON 470µF/35V × 4 모터별)

### 2. 버스 TVS (최후 클램프)

벌크 캡만으로 안심 못 함 — 빠른 dV/dt (L·dI/dt 스파이크) 는 캡이 못 따라옴.

```
Bus TVS: P6KE33CA × 2 병렬 (1,200W 총 흡수 능력)
  작동 임계: 33V
  클램프 전압: 37.8V @ 18A
  AK 모터 내압 40V > 37.8V → 안전 마진 ✅
```

### 3. 브레이크 저항 회로 (능동 소산)

V_bus 가 26.5V 초과하면 **비교기가 MOSFET 켜서 저항으로 에너지 버림**.

→ [[Brake Resistor Circuit]] 에 상세

### 4. 모터별 로컬 보호

각 모터 커넥터 바로 앞에:
- SMAJ28CA (400W TVS, 28V 임계) — 다른 모터로 전파 차단
- 470µF/35V OS-CON (로컬 흡수)
- BLM31AG 페라이트 비드 (고주파 격리)

## 우리 보드에서의 적용 체크리스트

- [ ] Bus bulk: Rubycon 35ZLH4700M × 2 (입력 커넥터 3mm 이내)
- [ ] Bus TVS: P6KE33CA × 2 병렬
- [ ] Brake circuit: TLV3201 + IPB025N10N3 + 3Ω/50W ([[Brake Resistor Circuit]])
- [ ] 모터당 로컬 보호 4세트
- [ ] Bulk 캡 GND via 는 패드 직결 (ESL 최소)
- [ ] 입력 커넥터 → 벌크 캡 트레이스 < 5mm (인덕턴스 최소)

## 48V 시스템으로 갈 때 재계산 (Phase B)

24V 기준 현재 설계. 48V 로 올리면:

- 배터리 완충: 50.4V
- V_limit 재설정 필요 (예: 60V)
- 벌크 캡 V 정격: **63V 또는 100V** ([[Voltage-Current Scaling]])
- P6KE33CA → **P6KE68CA** 같은 더 높은 임계
- Brake 임계: 26.5V → 약 55V

현재 부품 그대로 48V 로 올리면 **벌크 캡 터짐** — 반드시 재선정.

## PCB 레이아웃 규칙

### 고전류 루프 면적 최소화

```
❌ 잘못: 배터리 커넥터 → 100mm 트레이스 → 캐패시터 → 모터
    (루프 면적 큼 → L 증가 → V 스파이크 증가)

✅ 올바름: 배터리 커넥터 옆 3mm 이내에 벌크 캐패시터 배치
    벌크 GND via 는 캐패시터 패드 바로 직결 (별도 트레이스 없이)
```

### 모터 리턴 경로 전용화

- 모터 커넥터 GND 핀 → via 4개 이상 → Layer 2 PGND 플레인 → 배터리 GND
- 이 경로에 MCU/센서 GND via 들어가지 않게

## 관련

- [[Brake Resistor Circuit]]
- [[Inrush Current Limiting]]
- [[Motor Power Isolation]]
- [[Motor Power]] (schematic 블록)
- [[RECONCILIATION]]
- [[_legacy/EXOSUIT_PROTECTION]]
