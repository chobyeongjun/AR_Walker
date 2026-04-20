---
aliases: [Size Budget]
tags: [project, layout, status/provisional]
---

# Size Budget (잠정)

> ⏸️ **잠정 수치** — 구체 보드 사이즈는 **모터 확정 후 P6 레이아웃 단계** 에서 최종 결정.
> 사용자 방침 (2026-04-20): "보드 사이즈도 그냥 그 정도 생각해보고 진행을 하거나. motor 관련 정해지면 그렇게 진행."

보드 면적 예산 **추정**. 자세한 스택업은 [[PCB Stackup 6-layer]].

## 부품별 면적 (6L 양면 실장 가정)

| 블록 | 면적 (대략) | 비고 |
|---|---:|---|
| [[STM32H743VIT6]] LQFP100 (14×14) | 196 mm² | 데이터시트 DS12110 기반 |
| [[ESP32-C3-MINI-1U]] + IPEX | 219 mm² | Espressif 데이터시트 (13.2×16.6) + IPEX |
| [[DM3AT microSD]] | 210 mm² | Hirose 데이터시트 (~14×15) |
| [[ADS131M04]] TQFP-32 + [[REF5025]] | 80 mm² | TI SBAS950 (9×9 TQFP-32) + REF SO-8 |
| DC 입력 XT60 + 출력 XT30×2 (Phase B) | 215 mm² | 벤더 XT60/30 도면 |
| 모터 전원 보호회로 | 200 mm² | `_legacy/EXOSUIT_PROTECTION.md` 부품 면적 합 |
| 30A 트레이스 폴리곤 마진 | 100 mm² | IPC-2221 전류-폭 기준 |
| Buck (TPS62933 또는 AP63205WU) + 인덕터·캡 | 100 mm² | TI/Diodes 데이터시트 |
| LDO × 2 (TLV75533) + 캡 | 50 mm² | TI SBVS244 |
| USB-C 커넥터 | 80 mm² | GCT USB4105 도면 |
| [[ISO1050]] + 격리 DC/DC | 150 mm² | TI SLLS983 (SOIC-16 wide) + Murata MGJ2D (SIP-7) |
| [[AS5048A]] × 2 | 60 mm² | ams 데이터시트 (TSSOP-14) |
| JST-GH × 5 (CAN, Jetson, EBIMU, Loadcell×2, E-stop) | 150 mm² | JST GH 시리즈 도면 |
| [[E-stop Dual Safety\|E-stop 회로]] ([[74LVC1G08]] 등) | 25 mm² | |
| 디커플링·패시브 | 300 mm² | 양면 분산 |
| [[INA228]] × 2 + shunt + NTC | 60 mm² | TI SBOS882 (VSSOP-10) × 2 |
| **부품 합계** | **~2200 mm²** | |
| 양면 분산 시 한 면당 | ~1100 mm² | |
| × 1.3 (라우팅 마진) | ~1430 mm² | |
| **보드 면적 필요 (추정)** | **~2200 mm²** | 양면 실장 기준 |

## 잠정 목표 (확정 아님)

- **1차 (여유):** 50 × 50 mm = 2500 mm² · 6-layer · 양면 실장
- **공격적:** 45 × 45 mm = 2025 mm² · 8-layer

**🚩 확정은 P6 레이아웃 단계** — 모터/드라이버/커넥터 확정 후 실제 풋프린트 배치해보고 결정.

## 왜 6L 가 sweet spot (layer 수는 유지)

- 4L 60×60 → wearable 부적합 (사용자 "컴팩트" 요구)
- 6L 50×50 (잠정) → wearable 적합 · 비용 차이 작음
- 8L 45×45 → 5mm 추가 축소 가능, 모터 전원 dual plane 유리
- PCB 단가 (5장 기준, JLCPCB 기준치): 4L ~$15, 6L ~$45, 8L ~$70

출처: 사용자 경험 / JLCPCB 공개 가격표 (2024~2025 estimate, 발주 시 재확인).

## 변경 이력

- v3.5: 내가 50×50mm 제시
- **v3.9: "잠정" 으로 격하** — 사용자 "motor 정해지면 재결정" 방침

## 관련

- [[PCB Stackup 6-layer]]
- [[BGA vs LQFP]]
- [[Block Diagram]]
- [[Citations & Sources]]
