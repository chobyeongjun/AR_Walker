# STM_Board — Exosuit Custom STM32 Board

> **Obsidian vault** for cable-driven exosuit custom STM32 board design.
> Open this folder in Obsidian → start from `[[00 Index]]` (or `00 Index.md`).

## Quickstart (Obsidian)

1. Obsidian → "Open folder as vault" → select `STM_Board/`
2. Open `00 Index.md` as the entry point
3. Use **Graph View** (`Ctrl+G`) to see note connections
4. Use **Templates** plugin with `90 Templates/` as template folder

## Structure

```
00 Index.md                    ← start here
10 Project/                    ← goals, plan, decisions, open Qs
20 Components/                 ← atomic notes per IC/part
30 Concepts/                   ← engineering concepts (eFuse, SYNC, ...)
40 Schematic Blocks/           ← hierarchical sheet work notes
50 References/                 ← datasheets, app notes, ref boards
60 BOM/                        ← bill of materials
70 KiCad/                      ← the actual KiCad project
80 Journal/                    ← daily/session notes (yours)
90 Templates/                  ← reusable note templates
Assets/                        ← images, diagrams
.obsidian/                     ← Obsidian config
```

## Local mirror

```bash
ln -s ~/AR_Walker/STM_Board ~/stm_board   # already done
```

## Branch

`claude/exosuit-board-design-LiIxS` — all work committed here.
