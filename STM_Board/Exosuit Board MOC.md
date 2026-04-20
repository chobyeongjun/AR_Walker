---
aliases: [Index, Home, Root MOC]
tags: [moc, index]
---

# Exosuit STM32 Board — Second Brain

> Cable-driven Exosuit용 커스텀 STM32 보드 설계 vault.
> 모든 노트는 atomic 단위 + `[[wikilinks]]` 로 연결. Graph View 로 전체 구조 확인.

## 🗺️ Maps of Content

- [[Project MOC]] — 프로젝트 전체 계획·상태·결정
- [[Components MOC]] — 부품별 atomic 노트
- [[Concepts MOC]] — 재사용 가능한 엔지니어링 개념
- [[Schematic Blocks MOC]] — hierarchical sheet 작업 노트
- [[Reference Index]] — 외부 데이터시트·앱노트·보드
- [[BOM MOC]] — Bill of Materials

## 🎯 현재 상태

- **Phase**: P2 진입 준비 (사용자 승인 대기)
- **MCU**: [[STM32H723VGT6]] 확정
- **보드 사이즈**: 50 × 50 mm, [[PCB Stackup 6-layer|6-layer]]
- **전략**: [[Modular Phase A-B Strategy|Phase A (모터 무관) 먼저 freeze, Phase B (모터 의존) 보류]]
- **다음**: Phase A KiCad 프로젝트 생성 + [[Loadcell Amp|로드셀 앰프]] schematic 부터

## 🔗 빠른 링크

- [[Goals & Requirements]]
- [[Block Diagram]]
- [[Phase Plan]]
- [[Decisions Log]]
- [[Open Questions]]
- [[Size Budget]]

## 🧭 네비게이션 규칙

1. 부품 질문 → `20 Components/`
2. 개념 / 설계 선택 근거 → `30 Concepts/`
3. 실제 회로 작업 → `40 Schematic Blocks/`
4. 외부 자료 → `50 References/`
5. 부품 리스트 → `60 BOM/`
6. KiCad 프로젝트 → `70 KiCad/`
7. 작업일지 → `80 Journal/`
8. 새 노트 만들 때 → `90 Templates/` 에서 복사

## 🏷️ 태그 체계

- `#type/component` — 부품 노트
- `#type/concept` — 개념 노트
- `#type/decision` — 결정 기록
- `#type/block` — schematic 블록
- `#status/open` `#status/decided` `#status/deferred`
- `#domain/power` `#domain/mcu` `#domain/sensor` `#domain/comm` `#domain/safety`
- `#phase/A` (모터 무관) `#phase/B` (모터 의존)
