---
aliases: [BOM]
tags: [moc, bom]
---

# BOM MOC

## 최신 BOM

- **[[BOM_v4]]** — 2026-04-20 v3.9 반영 (SYNC 제거)

## 히스토리

| 버전 | 날짜 | 주요 변경 |
|---|---|---|
| v0 | 2026-04-17 | 초기 scaffold |
| v1 | 2026-04-17 | ADS131M04, ESP32, H7 전환 |
| v2 | 2026-04-20 오전 | Obsidian 리팩토링 + Phase A/B 분리 |
| v3 | 2026-04-20 오후 | RECONCILIATION: H723→H743, TCAN1462→ISO1050, 보호 아키텍처, 2× INA228 |
| **v4** | 2026-04-20 저녁 | **v3.9 반영: SYNC 제거, Size 잠정, CubeMX 핀 태그** |

## 구조

CSV 컬럼: `block, ref, part, package, qty, source, notes, phase, decided`

- `phase`: A (모터 무관) / B (모터 의존)
- `decided`: yes / no / tbd

## v4 에서 제거된 것

- J_SYNC (JST-GH 3pin for Hardware SYNC GPIO) — 사용자 제거 결정
- LED_SYNC — SYNC 신호 없으니 무의미

## 발주 전 체크

- [ ] 각 부품 LCSC/Mouser/Digikey 재고·단가 확인 (BOM 의 가격은 내 기억 기반 2024~2025 대략값)
- [ ] 대체 부품 (second source) 기록
- [ ] KiCad 풋프린트 매칭 확인
- [ ] 패시브 (R/C) 정리 (0402 통일 등)
- [ ] Phase B 모터 확정 후 `decided=tbd` 항목 갱신

## Phase B 이후 확정

- 커넥터 XT60 vs XT90 vs XT30 (모터 전류 확정 후)
- Buck AP63205WU-7 (24V) vs TPS62933 (48V 대응)
- 퓨즈 정격 15A/30A/40A
- 배터리 V 의존 부품 (48V 면 SMAJ56CA, P6KE68CA 등)

## 관련

- [[BOM_v4]]
- [[Reference Index]] — 데이터시트 URL
- [[Citations & Sources]] — 가격·핀 할당 불확실 영역 명시
