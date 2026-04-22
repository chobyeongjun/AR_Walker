---
aliases: [P6KE33CA]
tags: [type/component, domain/power, domain/safety, phase/B, status/decided]
type: component
part_number: P6KE33CA
package: DO-15 (through-hole, axial)
manufacturer: Vishay
voltage_range: 33V standoff
interface: bidirectional TVS
phase: B
status: decided
---

# P6KE33CA

## 한 줄 요약

600W 양방향 TVS — 버스 전압 TVS 클램프 (2개 병렬 → 1,200W), AK 모터 내압 40V 이내에서 최후 보호.

## 사양

- 양방향 (CA suffix)
- 스탠드오프 전압: 33V
- 브레이크다운: 36.7~40.7V
- 클램프 전압 @ 18A: 45.7V (단일), **37.8V @ 9A (병렬 2개 = 18A)**
- 파워: 600W (단일), 1,200W (병렬 2개)
- 패키지: DO-15 axial

## 왜 선택했나

- 버스 정상 24V (완충 25.2V) < 33V 스탠드오프 → 평시 OFF
- 서지 시 37.8V 로 클램프 → AK 모터 내압 40V 이내 OK
- 병렬 2개로 1,200W 에너지 흡수 (회생 + dV/dt 스파이크 동시 대응)
- [[Brake Resistor Circuit]] (26.5V 임계) 로 잡지 못한 고주파 서지 최후 방어

## 연결

- [[Motor Power]] — 버스 부근 배치

## 관련

- [[Regen Energy Protection]]
