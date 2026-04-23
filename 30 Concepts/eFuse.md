---
aliases: [eFuse, electronic fuse]
tags: [type/concept, domain/power, domain/safety]
type: concept
---

# eFuse (Electronic Fuse)

## 문제

모터 전원이 보드를 통과할 때 필요한 보호:
- 회생 전류 (감속 시 24V→30V+ 솟음)
- 케이블 단락 (μs 단위 응답 필요)
- 핫 플러그 inrush 전류 (100A+)
- 역접속 (24V 거꾸로 꽂으면 모든 부품 사망)
- 모터 stall 과전류
- 배터리 brownout (저전압 → 비정상 동작)

개별 부품 (퓨즈 + 다이오드 + TVS + 비교기 + soft-start) 으로 만들면 7~10개 부품, 튜닝 까다로움, 응답 느림.

## 해법

**eFuse = 한 IC + 외부 MOSFET 1개로 위 기능 전부 통합.**

| 기능 | 개별 회로 | eFuse |
|---|---|---|
| OVP | 비교기+Zener+MOSFET driver | 저항 2개로 설정 |
| UVLO | 히스테리시스 비교기 | 저항 2개 |
| RVP | P-MOSFET ideal diode 회로 | 동일 외부 MOSFET 재활용 |
| Inrush limit | NTC + soft-start | 캡 1개 (ramp 시간) |
| 과전류 | 션트 + 비교기 | 내장 션트 측정 |
| Power good | 별도 비교기 | 핀 1개 → MCU EXTI |
| Fault latch | RS 래치 | 핀 설정으로 선택 |

**응답 속도:** 일반 퓨즈 ms → eFuse **μs** (1000배 빠름).
**면적:** 개별 ~200mm² → eFuse ~50mm².

## 우리 보드에서의 적용

부품:
- **[[LTC4368-1]]** — 0~80V 지원 (우리 24~48V 모두 커버)
  - latch 모드 (-1) — OVP/OCP 발생 시 reset 필요, 명시적 알림
- **[[IPB180N06S4]]** — 60V/180A N-MOSFET, 15A+ 통과

설정:
- OVP: ~28V (24V 시) 또는 ~52V (48V 시) — [[Voltage-Current Scaling]]
- UVLO: 18V
- OCP: ~35A
- Inrush ramp: 10 ms

## 비유

가전 멀티탭의 **surge protector + GFCI + breaker + soft-start** 가 하나의 IC 로.

## 왜 TPS25940 이 아닌가

- TPS25940 은 4~18V — 우리 전압에 부족
- LTC4368 은 0~80V — 미래 전압 변경에도 안전

## 관련 부품

- [[LTC4368-1]]
- [[IPB180N06S4]]
- [[INA228]] (병행 측정)

## 관련 개념

- [[Motor Power Isolation]]
- [[Voltage-Current Scaling]]

## 관련 결정

- [[Decisions Log]] — eFuse 채택 (2026-04-17)
