---
aliases: [SMAJ28CA]
tags: [type/component, domain/power, domain/safety, phase/B, status/decided]
type: component
part_number: SMAJ28CA
package: SMA (SMD)
manufacturer: Vishay
voltage_range: 28V standoff
interface: bidirectional TVS
phase: B
status: decided
---

# SMAJ28CA

## 한 줄 요약

400W 양방향 TVS — 모터별 로컬 보호 × 4. 한 모터 서지가 다른 모터로 전파 차단.

## 사양

- 양방향 (CA suffix)
- 스탠드오프 전압: 28V
- 클램프: 45.4V @ 8.8A
- 파워: 400W
- 패키지: SMA SMD (소형)

## 배치

각 모터 커넥터 바로 앞 (3mm 이내):

```
Bus ─── [SMAJ28CA] ── [470µF OS-CON] ── [BLM31AG ferrite] ── 모터 커넥터
         │
        GND
```

## 왜 4개

4 모터 각각 독립 보호. 한 모터 케이블 단락이나 회생 서지가 다른 모터·MCU 로 전파되지 않음.

## 관련

- [[Regen Energy Protection]]
- [[Motor Power]]
