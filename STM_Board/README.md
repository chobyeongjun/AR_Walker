# STM_Board — Exosuit Custom STM32 Board

> Obsidian 호환 **프로젝트 모듈**. 단독 vault 아님 — 사용자의 기존 `~/research-vault/` 에 symlink 로 편입해서 사용.

## 구조

```
Exosuit Board MOC.md           ← 프로젝트 루트 MOC (시작점)
10 Project/                    ← goals, plan, decisions, open Qs
20 Components/                 ← atomic notes per IC/part
30 Concepts/                   ← engineering concepts (eFuse, SYNC, …)
40 Schematic Blocks/           ← hierarchical sheet work notes
50 References/                 ← datasheets, app notes, ref boards
60 BOM/                        ← bill of materials (v2.csv)
70 KiCad/                      ← KiCad project (scaffold TBD)
80 Journal/                    ← project-specific session notes
90 Templates/                  ← project-specific templates (Schematic Block)
Assets/                        ← images, diagrams
```

## 사용자 vault 에 연결

```bash
# 기존 ~/research-vault/ 가 있다면 symlink 만 걸면 됨
ln -s ~/AR_Walker/STM_Board ~/research-vault/stm-board
```

그 후 Obsidian 에서 `~/research-vault/` 를 vault 로 열면 `stm-board/` 폴더로 이 모든 노트 접근.

## 공용 템플릿 안내

이 프로젝트는 3개 공용 템플릿 (Component, Concept, Decision) 을 **vault 루트의 `templates/` 에 위치하는 것을 가정**하고 만듦 (사용자 기존 구조 존중).
없으면 `90 Templates/` 에서 가져다 쓰면 됨 (이전 버전 백업).

프로젝트 전용 `Schematic Block` 템플릿은 `90 Templates/` 에 유지.

## 로컬 경로 (3종, 모두 동일 파일)

| 경로 | 용도 |
|---|---|
| `~/stm_board/` | **일상 작업** (사용자 선호) |
| `~/research-vault/stm-board/` | Obsidian |
| `~/AR_Walker/STM_Board/` | git 원본 |

## Git

- 레포: `chobyeongjun/ar_walker`
- 브랜치: `claude/exosuit-board-design-LiIxS`
- 편집은 어느 경로로 해도 같이 반영 (symlink 이므로).
