---
aliases: [Size Budget]
tags: [project, layout]
---

# Size Budget

보드 면적 예산. 자세한 스택업은 [[PCB Stackup 6-layer]].

## 부품별 면적 (6L 양면 실장 가정)

| 블록 | 면적 (대략) | 비고 |
|---|---:|---|
| [[STM32H723VGT6]] LQFP100 (14×14) | 196 mm² | 확정 |
| [[ESP32-C3-MINI-1U]] + IPEX | 219 mm² | IPEX 라 keep-out 작음 |
| [[DM3AT microSD]] | 210 mm² | 필수 |
| [[ADS131M04]] TQFP-32 + [[REF5025]] | 80 mm² | |
| DC 입력 XT60 + 출력 XT30×2 (Phase B) | 215 mm² | [[Voltage-Current Scaling]] |
| 모터 전원 보호회로 ([[eFuse|LTC4368]] + MOSFET + 470µF×4 + choke) | 200 mm² | Phase B |
| 30A 트레이스 폴리곤 마진 | 100 mm² | 라우팅 여유 |
| [[TPS62933]] Buck + 인덕터·캡 | 100 mm² | |
| LDO × 2 + 캡 | 50 mm² | |
| USB-C 커넥터 | 80 mm² | |
| [[TCAN1462]] + ESD | 30 mm² | |
| [[AS5048A]] × 2 | 60 mm² | |
| JST-GH × 6 | 180 mm² | CAN, Jetson, SYNC, EBIMU, Loadcell×2, Estop |
| [[E-stop Dual Safety|E-stop 회로]] ([[74LVC1G08]] 등) | 25 mm² | |
| 디커플링·패시브 | 300 mm² | 양면 분산 |
| [[INA228]] + shunt + NTC | 40 mm² | |
| **부품 합계** | **~2150 mm²** | |
| 양면 분산 시 한 면당 | ~1100 mm² | |
| × 1.3 (라우팅 마진) | ~1430 mm² | |
| **보드 면적 필요** | **~2200 mm²** | |

## 목표

- **1차 (추천):** 50 × 50 mm = **2500 mm²** · 6-layer · 양면 실장
- **공격적:** 45 × 45 mm = 2025 mm² · 8-layer · 양면 실장 + 모터 전원 dual plane

## 왜 6L / 50×50 이 sweet spot

- 4L 60×60 → wearable 부적합
- 6L 50×50 → 이득 크고 비용 작음 (PCB $45/장 vs $15/장, 5장 기준 +$150)
- 8L 45×45 → 5mm 더 줄어듬 (+$25/장), 모터 전원 평면 dual
- HDI 40×40 → 시제품엔 과잉

## 관련

- [[PCB Stackup 6-layer]]
- [[BGA vs LQFP]]
- [[Block Diagram]]
