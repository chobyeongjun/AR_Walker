# KiCad Project

KiCad 9 (또는 8) 프로젝트가 여기 들어감. [[Phase Plan]] 의 **P2** 에서 생성.

## 구조 (계획)

```
70 KiCad/
├── exo_stm32.kicad_pro              # 프로젝트 파일
├── exo_stm32.kicad_sch              # 루트 schematic (hierarchical)
├── exo_stm32.kicad_pcb              # PCB 레이아웃
├── sheets/                          # hierarchical sub-sheets
│   ├── logic_power.kicad_sch
│   ├── mcu_core.kicad_sch
│   ├── loadcell_amp.kicad_sch       # ⭐ 우선
│   ├── encoder.kicad_sch
│   ├── comms.kicad_sch
│   ├── wireless.kicad_sch
│   ├── storage.kicad_sch
│   ├── usb_debug.kicad_sch
│   ├── safety_ui.kicad_sch
│   ├── battery_monitor.kicad_sch
│   └── motor_power.kicad_sch        # Phase B
├── symbols/                         # 커스텀 심볼 (없는 부품)
├── footprints/                      # 커스텀 풋프린트
└── 3dmodels/                        # STEP 파일
```

## 연결

- [[Schematic Blocks MOC]]
- [[Phase Plan]]
