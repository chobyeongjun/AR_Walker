---
aliases: [BOM]
tags: [moc, bom]
---

# BOM MOC

## 최신 BOM

- [[BOM_v2]] — CSV, Phase A 부품 확정 + Phase B placeholder
- `BOM_v2.csv` — 기계 판독용

## 히스토리

- v0 (4월 17일) — 초기 scaffold
- v1 (4월 17일) — ADS131M04, ESP32, H7 전환
- **v2 (4월 20일) — Obsidian 리팩토링 + Phase A/B 분리**

## 구조

CSV 컬럼: `block, ref, part, package, qty, source, notes, phase, decided`

- `phase`: A (모터 무관) / B (모터 의존)
- `decided`: yes / no / tbd

## 발주 전 체크

- [ ] 각 부품 LCSC/Mouser/Digikey 재고·단가 확인
- [ ] 대체 부품 (second source) 기록
- [ ] KiCad 풋프린트 매칭 확인
- [ ] 패시브 (R/C) 정리 (0402 통일 등)
