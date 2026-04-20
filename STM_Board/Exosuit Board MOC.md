---
aliases: [Exosuit Board MOC, Exosuit Board, STM_Board, Cable-Driven Exosuit]
tags: [moc, project, domain/robotics, domain/electronics]
---

# Exosuit Board MOC

> Cable-driven Exosuit 용 커스텀 STM32 보드 프로젝트 루트 MOC.
> 이 폴더는 `~/AR_Walker/STM_Board/` git repo 이며, `~/research-vault/stm-board` symlink 로 vault 에 편입.

## 🎯 현재 상태 (v3.5 reconciliation 반영)

- **Phase**: P2 진입 준비
- **MCU**: [[STM32H743VIT6]] (H723 에서 변경, [[RECONCILIATION]])
- **제어 루프**: 500 Hz (어드미턴스 + 위치 PID)
- **배터리**: 6S Li-ion 25.2V 완충 (48V 직렬 옵션은 Phase B)
- **CAN**: **[[ISO1050]] 격리** (GND bounce 파손 방지)
- **로드셀**: [[ADS131M04]] (32 kSPS 동시 샘플링)
- **IMU**: EBIMU EBMotion V5 (무선)
- **카메라/Jetson**: ZED X Mini + Jetson Orin NX 16GB
- **보드 사이즈**: 50 × 50 mm, [[PCB Stackup 6-layer|6-layer]]
- **전략**: [[Modular Phase A-B Strategy]]
- **보호 아키텍처**: [[Regen Energy Protection]] + [[GND Bounce Protection]] + [[Isolated CAN]] + [[Brake Resistor Circuit]] + [[Inrush Current Limiting]]
- **다음**: Phase A 데이터시트 수집 → KiCad 프로젝트 → [[Loadcell Amp]] sheet

## 🗺️ 서브 MOC

- [[Project MOC]] — 목표·계획·결정
- [[Components MOC]] — 부품별 atomic 노트
- [[Concepts MOC]] — 엔지니어링 개념
- [[Schematic Blocks MOC]] — hierarchical sheet 작업 노트
- [[Reference Index]] — 외부 데이터시트·앱노트·보드
- [[BOM MOC]] — Bill of Materials
- [[RECONCILIATION]] — Legacy vs 내 노트 diff (v3.5 핵심 문서)

## 🔗 빠른 링크

- [[Goals & Requirements]]
- [[Block Diagram]]
- [[Phase Plan]]
- [[Decisions Log]]
- [[Open Questions]]
- [[Size Budget]]
- [[Modular Strategy]]

## 📁 Legacy 문서 (2026-04-12~14, 이전 Claude 세션 결과)

`10 Project/_legacy/` 에 보존:
- [[_legacy/BOARD_PLAN|BOARD_PLAN]] — 초기 5-Phase 마스터 플랜
- [[_legacy/BOARD_DESIGN|BOARD_DESIGN]] — 상세 회로 초기본
- [[_legacy/BOARD_DESIGN_REVIEWED|BOARD_DESIGN_REVIEWED]] — **4-agent 검토 결과 (CRITICAL 7 + WARNING 15)**
- [[_legacy/EXOSUIT_PROTECTION|EXOSUIT_PROTECTION]] — **보호 회로 최종 설계**
- [[_legacy/HANDOFF|HANDOFF]] — 인수인계 문서
- [[_legacy/RECONCILIATION|RECONCILIATION]] — legacy vs 내 노트 통합

## 🧭 프로젝트 내 폴더

1. `10 Project/` — 목표·계획·결정 (legacy 포함)
2. `20 Components/` — 부품 atomic 노트
3. `30 Concepts/` — 엔지니어링 개념 (보호 아키텍처 포함)
4. `40 Schematic Blocks/` — KiCad sheet 작업 노트
5. `50 References/` — 데이터시트
6. `60 BOM/` — BOM CSV + MOC
7. `70 KiCad/` — 실제 KiCad 프로젝트
8. `80 Journal/` — 프로젝트 작업일지
9. `90 Templates/` — 프로젝트 전용 템플릿

## 🏷️ 태그

- `#type/component` · `#type/concept` · `#type/decision` · `#type/block`
- `#domain/power` · `#domain/mcu` · `#domain/sensor` · `#domain/comm` · `#domain/safety`
- `#status/decided` · `#status/rejected` · `#status/deferred`
- `#phase/A` (모터 무관) · `#phase/B` (모터 의존)

## 📁 경로 3종 (모두 동일 파일)

| 경로 | 용도 |
|---|---|
| `~/stm_board/` | 일상 작업 |
| `~/research-vault/stm-board/` | Obsidian |
| `~/AR_Walker/STM_Board/` | git 원본 |

- 브랜치: `claude/exosuit-board-design-LiIxS`
