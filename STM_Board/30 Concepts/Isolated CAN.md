---
aliases: [Isolated CAN, 격리 CAN]
tags: [type/concept, domain/comm, domain/safety]
type: concept
---

# Isolated CAN

## 문제

이전 exosuit 보드가 **반복적으로 CAN 트랜시버 파손** → 시스템 다운.

원인 분석 ([[EXOSUIT_PROTECTION]] 수치):

```
모터 리턴 전류 급변:
  dI/dt ≈ 50 A/µs
PCB GND 인덕턴스:
  L_pcb ≈ 50 nH

V_bounce = L × dI/dt = 50nH × 50A/µs = 2.5 V

CAN 트랜시버 (비격리) TXD/RXD 절대최대정격:
  VIO ± 0.3V

실제 bounce 2.5V → 절대최대 8× 초과 → 즉시 파손
```

→ 비격리 CAN 은 **exosuit 환경에 구조적으로 부적합.**

## 해법: 갈바닉 격리 (Galvanic Isolation)

MCU 측 GND 와 CAN 버스 측 GND 를 **물리적으로 끊어버림.**

```
[MCU 3.3V AGND 도메인]        [CAN 버스 5V PGND 도메인]
                                (모터 전원 GND, bounce 발생)

FDCAN_TX ─┐                   ┌─ CANH
FDCAN_RX ─┤     ISO1050       ├─ CANL
3.3V ─────┤   (5000 Vrms)     ├─ 5V ← MGJ2D05050SC 격리 공급
AGND ─────┘                   └─ PGND

          ═══ 격리 경계 ═══
          (6mm creepage, 솔더마스크 갭 2mm)
```

격리 내압 5000 Vrms → 2.5V bounce 완전 무해.

## 우리 보드에서의 적용

### 부품
- **[[ISO1050]]** — 격리 CAN 트랜시버 (CANopen 클래식 CAN 1 Mbps)
- **[[MGJ2D05050SC]]** — 격리 DC/DC (VCC2 5V 독립 공급)
- **BZT52C5V6** — VCC2 5.6V 제너 클램프 (절대최대 6.5V 보호)
- **ACM2012-900** — 공통모드 초크 (EMI 차단, 900Ω@100MHz)
- **SM712-02HTG** — CAN 버스 TVS 500W (±8 kV ESD)
- **PRTR5V0U2X** — MCU 측 FDCAN 핀 ESD (추가 보호)

### 회로

```
STM32H743 FDCAN1
     │
 [33Ω × 2] + PRTR5V0U2X ESD
     │
 ISO1050 VCC1 (3.3V AGND) ─┐
                            │
                       ═══ 격리 ═══
                            │
 ISO1050 VCC2 (5V PGND) ◄── MGJ2D05050SC (격리 5V)
                            │
                      [BZT52C5V6 5.6V 제너]
     │
 [22Ω 댐핑] + ACM2012-900 CMC
     │
 SM712-02HTG TVS
     │
 CAN 커넥터 → Elmo 드라이버 (120Ω 종단, 데이지체인)
```

## 트레이드오프

| 항목 | 비격리 ([[TCAN1462]]) | 격리 (ISO1050) |
|---|---|---|
| GND bounce 내성 | ❌ 2.5V 에 파손 | ✅ 5000 Vrms |
| 부품 수 | 1 IC | ISO1050 + 격리 DC/DC + 제너 + 더 큰 TVS |
| 면적 | ~30 mm² | ~150 mm² |
| 비용 | ~$2 | ~$15 (ISO1050 $4 + MGJ2D $10 + 기타) |
| 데이터 속도 | CAN-FD 5Mbps | 클래식 CAN 1Mbps (ISO1042 면 FD 2Mbps) |
| 신뢰성 | 환경 의존 | 압도적 ✅ |

**Exosuit 같은 고전류 스위칭 환경에선 신뢰성이 모든 다른 지표 이김.**

## PCB 레이아웃 규칙

→ [[Motor Power Isolation]] 참고

- ISO1050 패키지가 AGND ↔ PGND 경계를 수직으로 가로지름
- Layer 1~N 모두 솔더마스크 갭 ≥ 2mm
- ISO1050 아래 Layer 2 GND 에도 갭 (via 금지)
- AGND/PGND 는 [[Motor Power Isolation|단일점 스타 그라운드]] 에서만 연결

## Legacy review 에서의 CAN 진화

| 단계 | CAN 트랜시버 | 보호 |
|---|---|---|
| 초기 | TJA1051 (비격리) | SMAJ5.0A |
| Review 후 | ISO1050 (격리) | BZT52C5V6 + 격리 DC/DC |
| 내 초기 추천 | TCAN1462 (비격리) | 일반 ESD |
| **최종** | **ISO1050 (격리)** | legacy review 동일 |

## 관련

- [[ISO1050]]
- [[MGJ2D05050SC]]
- [[GND Bounce Protection]]
- [[Motor Power Isolation]]
- [[Comms]]
- [[TCAN1462]] (rejected)
