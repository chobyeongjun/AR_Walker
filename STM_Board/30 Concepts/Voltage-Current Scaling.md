---
aliases: [Voltage Current Scaling, V-I 매트릭스]
tags: [type/concept, domain/power]
type: concept
---

# Voltage-Current Scaling

## 문제

모터 전압이 낮아질수록, 같은 전력에 **전류는 반비례로 증가**.
→ 보드 커넥터·트레이스·MOSFET 선정이 전압에 따라 달라짐.

## 전압별 부품 매트릭스

| 모터 전압 | 배터리 구성 | 벌크 캡 V 정격 | TVS | MOSFET V_DS | 동일 전력 시 입력 전류 |
|---|---|---|---|---|---|
| 12V | Li-ion 3S (11.1V) | 25V | SMCJ15A | 30~40V | ↑↑ (2~4배) |
| 24V | Li-ion 4S (14.8V) / 6S (22.2V) | 35V | SMCJ24A/30A | 40V | **기본 가정** |
| 36V | Li-ion 8S (29.6V) / 10S | 50V | SMCJ36A | 60V | ↓ |
| 48V | Li-ion 12S (44.4V) / 14S | 63V | SMCJ48A | 80V | ↓↓ |

## 중요 주의

**"15A 모터" 가 전류 기준인지 전력 기준인지 확인 필요.**

- 전류 기준 (예: "정격 15A") → 전압 변화해도 전류 동일, 커넥터 선정 무관
- 전력 기준 (예: "360W") → 전압 바뀌면 전류 변함:
  - 48V → 7.5 A
  - 24V → 15 A
  - 12V → 30 A ← 같은 전력에 두 배 커넥터 필요

## Phase A 영향 (없음 또는 작음)

이 부품들은 전압 결정 전이라도 freeze 가능:

- [[STM32H723VGT6]] — 3.3V
- [[ADS131M04]] — 3.3V
- [[INA228]] — **0~85V 전 범위 커버** (IC 자체는 전압 무관)
- [[LTC4368-1]] — **0~80V 전 범위**
- [[TPS62933]] — **0~60V 입력 커버** (토폴로지 freeze, 입력 캡만 Phase B)
- [[TLV75533]] — 5V→3.3V (무관)

## Phase B 영향 (전압 확정 후 선정)

- DC 입력 커넥터 (XT30 / XT60 / XT90) — 전류 의존
- 모터 출력 커넥터 — 전류 의존
- eFuse 외부 N-MOSFET V_DS — 전압 의존
- 벌크 캡 V 정격 — **전압 의존**
- TVS 클램프 전압 — **전압 의존**
- Common-mode choke 정격 — 전류 의존
- 백업 퓨즈 정격 — 전류 의존
- 모터 전원 폴리곤 너비 — 전류 의존

## 관련

- [[Modular Phase A-B Strategy]]
- [[Open Questions]] — 모터 모델·전압·전류 미확정
- [[Motor Power]]
