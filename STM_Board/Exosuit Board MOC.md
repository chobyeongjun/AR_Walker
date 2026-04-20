---
aliases: [Exosuit Board MOC, Exosuit Board, STM_Board, Cable-Driven Exosuit]
tags: [moc, project, domain/robotics, domain/electronics]
---

# Exosuit Board MOC

> Cable-driven Exosuit 용 커스텀 STM32 보드 프로젝트 루트 MOC.
> 이 폴더는 `~/AR_Walker/STM_Board/` git repo 이며, `~/research-vault/stm-board` symlink 로 vault 에 편입.

## 🎯 현재 상태 (v3.9)

- **Phase**: P1 (데이터시트 수집) 진행 중, P2 (KiCad) 준비 중
- **MCU**: [[STM32H743VIT6]]
- **제어 루프**: 500 Hz (어드미턴스 + 위치 PID)
- **배터리**: 6S Li-ion 25.2V 완충 (48V 직렬 옵션은 Phase B)
- **CAN**: **[[ISO1050]] 격리** (GND bounce 파손 방지)
- **로드셀**: [[ADS131M04]] (32 kSPS 동시 샘플링)
- **IMU**: EBIMU EBMotion V5 (무선)
- **카메라/Jetson**: ZED X Mini + Jetson Orin NX 16GB
- **무선**: [[ESP32-C3-MINI-1U]] BLE (v3.9 사용자 확정, 고수준 명령·텔레메트리·OTA 용)
- **커넥터**: JST-GH 1.25mm 통일 (v3.9 확정, JST 카탈로그 근거)
- **보드 사이즈**: ⏸️ **잠정 50×50mm** — motor 확정 후 P6 에서 결정
- **Layer 수**: 6-layer ([[PCB Stackup 6-layer]])
- **전략**: [[Modular Phase A-B Strategy]]
- **보호**: [[Regen Energy Protection]] + [[GND Bounce Protection]] + [[Isolated CAN]] + [[Brake Resistor Circuit]] + [[Inrush Current Limiting]]
- **제거됨** (v3.9): Hardware SYNC GPIO
- **다음**: `download_all.sh` → CubeMX 핀 확정 → KiCad 프로젝트

## 🗺️ 서브 MOC

- [[Project MOC]] — 목표·계획·결정·**출처** (Citations & Sources)
- [[Components MOC]] — 부품별 atomic 노트
- [[Concepts MOC]] — 엔지니어링 개념
- [[Schematic Blocks MOC]] — hierarchical sheet 작업 노트
- [[Reference Index]] — 외부 데이터시트·앱노트·보드
- [[BOM MOC]] — Bill of Materials

## 📋 투명성 (꼭 읽어)

- **[[Citations & Sources]]** ⭐ — 각 스펙·결정 출처 투명 기록. 사용자 방침: "너의 생각으로 하는 것들은 모두 X. 무조건 근거 있이."
- **[[Unsourced Items Audit]]** — 내 추가 13개 항목 감사, v3.9 답변 반영 완료

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

1. `10 Project/` — 목표·계획·결정·Citations (legacy 포함)
2. `20 Components/` — 부품 atomic 노트 (20+)
3. `30 Concepts/` — 엔지니어링 개념 (15)
4. `40 Schematic Blocks/` — KiCad sheet 작업 노트 (11)
5. `50 References/` — 데이터시트 (17 카테고리)
6. `60 BOM/` — BOM v4.csv (v3.9 SYNC 제거 반영)
7. `70 KiCad/` — 실제 KiCad 프로젝트 (P2)
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
