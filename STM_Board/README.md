# STM_Board — Exosuit Custom STM32 Board

> Obsidian 호환 프로젝트 모듈. **단독 vault 아님** — `~/research-vault/` 통합 vault 의 subfolder 로 symlink 되어 동작.

## 구조

```
10 Project/                    ← goals, plan, decisions, open Qs
20 Components/                 ← atomic notes per IC/part
30 Concepts/                   ← engineering concepts (eFuse, SYNC, …)
40 Schematic Blocks/           ← hierarchical sheet work notes
50 References/                 ← datasheets, app notes, ref boards
60 BOM/                        ← bill of materials (v2.csv)
70 KiCad/                      ← the actual KiCad project (scaffold TBD)
80 Journal/                    ← project-specific session notes
90 Templates/                  ← project-specific templates (Schematic Block)
Assets/                        ← images, diagrams
Exosuit Board MOC.md           ← project root MOC (starts here)
```

## Obsidian 에서 열기 (통합 vault)

1. `~/research-vault/` 를 Obsidian vault 로 열기
2. `00 Index.md` 에서 시작 → Projects MOC → [[Exosuit Board MOC]]
3. 또는 직접 `Projects/Exosuit_Board/Exosuit Board MOC.md` 열기

**공용 템플릿** (Component / Concept / Decision) 은 vault 루트의 `Templates/` 에 있음.
**프로젝트 전용 템플릿** (Schematic Block) 만 이 폴더의 `90 Templates/` 에 남아있음.

## 단독 vault 로 열고 싶다면 (비권장)

이 폴더를 Obsidian 에 vault 로 열어도 동작하지만:
- `Templates/Component`, `Templates/Concept`, `Templates/Decision` 깨진 링크
- `[[00 Index]]`, `[[Projects MOC]]`, `[[Areas MOC]]` 같은 상위 MOC 링크 깨짐

통합 vault 사용을 권장.

## 로컬 경로

- 원본: `~/AR_Walker/STM_Board/`
- 단축 symlink: `~/stm_board` (legacy, 유지)
- Obsidian 접근: `~/research-vault/Projects/Exosuit_Board/` (권장)

## Git

- 레포: `chobyeongjun/ar_walker`
- 브랜치: `claude/exosuit-board-design-LiIxS`
- 편집은 어느 경로로 해도 같이 반영됨 (symlink).
