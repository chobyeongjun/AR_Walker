---
aliases: [BGA vs LQFP, Package comparison]
tags: [type/concept, domain/mcu]
type: concept
---

# BGA vs LQFP

## 문제

[[STM32H743VIT6]] (LQFP100, 14×14) vs STM32H743VIH6 (UFBGA100, 7×7) 선택.
외주 PCBA 환경에서 어느 쪽이 좋은가?

## 성능 차이: 없음

- 동일 다이, CPU/RAM/Flash/주변기기 동일
- CoreMark, MFLOPS, 소비전력 동일
- SI 측면에서 BGA 가 미세하게 유리 (핀 길이 짧음) — 우리 USB HS 에서는 무관

## 실 비용 (외주 PCBA 가정)

| 항목 | LQFP100 | BGA UFBGA100 |
|---|---|---|
| 보드 면적 | 196 mm² | 49 mm² (-147) |
| PCB fab | 표준 4/6L | HDI microvia 필수 |
| PCB 비용 (5장) | $45 (6L) | +$30~60 |
| PCBA 비용 (5장) | 표준 SMT | X-ray 검수 필수, +$5~15/board |
| 디버깅 프로빙 | 핀 직접 | **불가** (TP 미리 깔아야) |
| 리워크 | 가능 | **사실상 불가** |
| Fab 리드타임 | 빠름 | HDI 때문에 +1~2주 |

**총 BGA 프리미엄: ~$50~100/board × 5장 = $250~500 추가**

## 보드 사이즈 관점

우리 보드의 사이즈 bottleneck:
1. [[DM3AT microSD]] — 210 mm²
2. [[ESP32-C3-MINI-1U]] — 219 mm²
3. 모터 전원 회로 ([[Motor Power]]) — ~500 mm²
4. 커넥터 (JST-GH × 6 + XT60 + USB-C) — ~400 mm²

**MCU 가 196 → 49 로 줄어도 보드는 거의 안 줄어듬.**
→ MCU 는 사이즈 bottleneck 아님.

## 언제 BGA 가 정답인가

1. 극단 컴팩트 (smartwatch 급 25×25mm) — 우리 아님
2. 양산 (1000장 이상) — 단가 차이 묻힘
3. 고속 USB 3.0 / DDR3 — 우리 USB HS 는 LQFP 충분

## 결론

**외주 조립이어도 LQFP100 추천 유지.**
시제품 단계에 BGA 의 비용·디버깅 부담이 사이즈 이점을 초과.
펌웨어는 100% 호환 → 양산 단계에 BGA 로 갈아끼울 수 있음.

## 관련

- [[STM32H743VIT6]]
- [[Size Budget]]
- [[Decisions Log]]
