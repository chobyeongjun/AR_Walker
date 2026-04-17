# STM_Board — Exosuit Custom STM32 Board

Cable-driven exosuit 정밀 토크 제어를 위한 커스텀 STM32 보드.
KiCad로 schematic → PCB → 시제품까지.

## 빠른 시작

1. `PLAN.md` 읽고 **§6 결정 필요 항목** 답변
2. `parts/BOM_v0.csv` 부품 후보 검토
3. `kicad/` 에서 KiCad 프로젝트 생성 (Phase 2)

## 폴더

| 폴더 | 용도 |
|---|---|
| `docs/` | 결정 기록, 다이어그램, 브링업 노트 |
| `kicad/` | KiCad 7+ 프로젝트 (`exo_stm32.kicad_pro` 등) |
| `parts/` | BOM, 데이터시트 링크 |
| `refs/` | 외부 참고 보드 자료 |
| `schematic_blocks/` | 블록별 작업 메모 |

## 로컬 거울

```bash
ln -s ~/AR_Walker/STM_Board ~/stm_board
```

## 브랜치

`claude/exosuit-board-design-LiIxS` 에서 개발.
