---
aliases: [Exosuit Board MOC, Exosuit Board, STM_Board, Cable-Driven Exosuit]
tags: [moc, project, domain/robotics, domain/electronics]
---

# Exosuit Board MOC

> Cable-driven Exosuit 용 커스텀 STM32 보드 프로젝트 루트 MOC.
> 이 폴더는 `~/AR_Walker/STM_Board/` git repo 이며, `~/research-vault/` vault 안에 symlink 로 포함됨.

## 🎯 현재 상태

- **Phase**: P2 진입 준비
- **MCU**: [[STM32H723VGT6]] 확정 (LQFP100)
- **보드 사이즈**: 50 × 50 mm, [[PCB Stackup 6-layer|6-layer]]
- **전략**: [[Modular Phase A-B Strategy|Phase A 먼저 freeze, Phase B 모터 확정 후]]
- **다음**: [[Reference Index]] 수집 → KiCad 프로젝트 → [[Loadcell Amp]] sheet

## 🗺️ 서브 MOC (프로젝트 내부)

- [[Project MOC]] — 목표·계획·결정 ([[Goals & Requirements]], [[Phase Plan]], [[Decisions Log]])
- [[Components MOC]] — 부품별 atomic 노트
- [[Concepts MOC]] — 엔지니어링 개념
- [[Schematic Blocks MOC]] — hierarchical sheet 작업 노트
- [[Reference Index]] — 외부 데이터시트·앱노트·보드
- [[BOM MOC]] — Bill of Materials

## 🔗 빠른 링크

- [[Goals & Requirements]]
- [[Block Diagram]]
- [[Phase Plan]]
- [[Decisions Log]]
- [[Open Questions]]
- [[Size Budget]]
- [[Modular Strategy]]

## 🧭 프로젝트 내 폴더

1. `10 Project/` — 목표·계획·결정
2. `20 Components/` — 부품 atomic 노트
3. `30 Concepts/` — 프로젝트 특화 개념 (재사용 가능하면 vault 의 `10_Wiki/` 로 승격 후보)
4. `40 Schematic Blocks/` — KiCad sheet 작업 노트
5. `50 References/` — 프로젝트 전용 데이터시트
6. `60 BOM/` — BOM CSV + MOC
7. `70 KiCad/` — 실제 KiCad 프로젝트 파일
8. `80 Journal/` — 프로젝트 작업일지
9. `90 Templates/` — 프로젝트 전용 템플릿 ([[Schematic Block]])

## 🏷️ 이 프로젝트에서 자주 쓰는 태그

- `#type/component` · `#type/concept` · `#type/decision` · `#type/block`
- `#domain/power` · `#domain/mcu` · `#domain/sensor` · `#domain/comm` · `#domain/safety`
- `#status/open` · `#status/decided` · `#status/deferred`
- `#phase/A` (모터 무관) · `#phase/B` (모터 의존)

## 📁 Git 추적

- 실제 위치: `~/AR_Walker/STM_Board/` (AR_Walker 레포)
- 브랜치: `claude/exosuit-board-design-LiIxS`
- vault 접근: `~/research-vault/stm-board/` (symlink, 사용자가 걸면 됨)

## 📝 Vault 통합 안내

이 프로젝트는 **단독 vault 아님** — 사용자의 기존 `~/research-vault/` (구조: `00_Raw/`, `10_Wiki/`, `20_Meta/`, `templates/`, `<project-name>/` …) 안에 symlink 로 편입.

통합 방법:
```bash
ln -s ~/AR_Walker/STM_Board ~/research-vault/stm-board
```

이 프로젝트의 `30 Concepts/` 중 재사용도 높은 것 (예: [[eFuse]], [[PCB Stackup 6-layer]], [[Loadcell Amplifier Design]]) 은 나중에 `~/research-vault/10_Wiki/` 로 승격하면 그래프 통합도 ↑.
