---
aliases: [GND Bounce, Ground Bounce]
tags: [type/concept, domain/pcb, domain/safety]
type: concept
---

# GND Bounce Protection

## 문제

모터 같은 고전류 스위칭 부하가 PCB GND 에 **빠르게 변하는 리턴 전류** 를 흘리면, GND 자체가 "진동" 해서 두 지점의 GND 전위가 달라짐.

### 수치 (이전 exosuit 보드 파손 사례)

```
모터 리턴 전류 변화율:  dI/dt = 50 A/µs
PCB GND 인덕턴스:       L = 50 nH (넓이 10cm² 루프 기준)
V_bounce = L × dI/dt = 50 × 10⁻⁹ × 50 × 10⁶ = 2.5 V
```

**결과:** 두 GND 지점이 순간 2.5V 차이 → GND 기준 신호가 전부 2.5V 상하로 흔들림.

### 무엇이 파손되나

| 부품 | 절대최대정격 | Bounce 영향 |
|---|---|---|
| [[TCAN1462]] TXD/RXD | VIO ± 0.3V | **즉시 파손** (8× 초과) |
| MCU GPIO | VDD + 0.3V | 래치업 위험 |
| ADC 입력 | VDDA + 0.3V | 측정값 왜곡 + 손상 |
| DC/DC 입력 캡 ESR | - | 열폭주 가속 |

## 해법 (계층적)

### 1. 근본 해결: 갈바닉 격리 (가장 강력)

고전류 부하 (CAN 버스) 와 MCU GND 를 **전기적으로 끊음**.

→ [[Isolated CAN]] ([[ISO1050]] + [[MGJ2D05050SC]])

→ Bounce 가 구조적으로 전파 불가. 이전 보드 파손 근본 원인 제거.

### 2. PCB 레이아웃: GND 임피던스 최소화

```
❌ 피해야 할 것
- 고전류 루프가 신호 영역 위로 지나감
- GND 플레인 분할 (AGND/DGND 물리 slit) — Henry Ott 금지
- 얇은 트레이스로 고전류 return

✅ 해야 할 것
- 단일 연속 GND 플레인 ([[PCB Stackup 6-layer]] L2, L5)
- 고전류 return 경로 폴리곤 굵게 (L 최소화)
- 2 oz 외층 동박 (30A 트레이스)
- 모터 영역을 한쪽에 몰아서 배치 ([[Motor Power Isolation]])
```

### 3. 구역 파티셔닝 (Henry Ott 방식)

GND 를 물리적으로 분할하지 **않고**, 대신 **부품 배치로 구역 나눔**:

```
┌──────────────┬──────────────┐
│   아날로그   │   디지털     │
│ 로드셀 + ADC │ MCU + CAN    │
│ [[Loadcell Amp]]│ [[MCU Core]]  │
│ (좌측 하단) │ (우측 전체)  │
└──────────────┴──────────────┘
규칙:
- 디지털 트레이스 ❌ 아날로그 영역 위 (Layer 1) 통과
- ADS131M04 AGND/DGND 핀은 단일 스타 포인트로
- 단일 연속 GND 플레인 유지 (자르지 않음)
```

### 4. Star ground 단일점

```
Motor PGND ────┐
               ●  ← 단 하나의 연결점 (입력 커넥터 근처)
MCU AGND  ────┘

연결 부품: [[BLM21PG601SN1D]] 600Ω@100MHz 페라이트 비드
```

고주파 노이즈는 격리, DC 는 공유.

## 우리 보드에서의 적용 체크리스트

- [ ] CAN 트랜시버: [[ISO1050]] (격리) 사용
- [ ] CAN 측 5V 공급: [[MGJ2D05050SC]] 격리 DC/DC
- [ ] Layer 2 + Layer 5 단일 연속 GND 플레인
- [ ] 모터 영역과 신호 영역 물리 분리 (PCB 상 좌우 또는 상하)
- [ ] PGND ↔ AGND 단일점 ([[Motor Power Isolation|star ground]])
- [ ] 2 oz 외층 동박 (30A 트레이스)
- [ ] 모터 트레이스 위 신호 라우팅 금지

## 왜 이 개념이 치명적인가

**Bounce 는 "측정 안 됨"** — 일반 DC 측정·오실로스코프 로는 놓치기 쉬움 (고주파, 순간적). 그래서 이전 보드에서 **왜 CAN 이 주기적으로 죽는지** 알아내는 데 오래 걸림.

격리 구조를 **처음부터** 넣는 게 정답.

## 관련

- [[Isolated CAN]]
- [[ISO1050]]
- [[Motor Power Isolation]]
- [[PCB Stackup 6-layer]]
- [[RECONCILIATION]]
