---
aliases: [Phase Plan, Phases]
tags: [project, plan]
---

# Phase Plan

전략은 [[Modular Strategy]] 참조 — Phase A (모터 무관, freeze) / Phase B (모터 의존, 보류).

## 단계

| Phase | 내용 | 산출물 | 상태 |
|---|---|---|---|
| **P0** | 요구·블록도 | [[Goals & Requirements]], [[Block Diagram]] | ✅ |
| **P1** | 각 블록 reference schematic 수집 | `50 References/Datasheets/`, [[Reference Index]] | 🟡 진행중 |
| **P2** | KiCad 프로젝트 생성 + 심볼/풋프린트 라이브러리 | `70 KiCad/exo_stm32.kicad_pro` | ⬜ |
| **P3A** | Phase A hierarchical sheet 작성 | per-block `.kicad_sch` | ⬜ |
| **P3A.1** | [[Logic Power]] | `logic_power.kicad_sch` | ⬜ |
| **P3A.2** | [[MCU Core]] | `mcu_core.kicad_sch` | ⬜ |
| **P3A.3** | **[[Loadcell Amp]]** (사용자 우선) | `loadcell_amp.kicad_sch` | ⬜ |
| **P3A.4** | [[Encoder]] | `encoder.kicad_sch` | ⬜ |
| **P3A.5** | [[Comms]] | `comms.kicad_sch` | ⬜ |
| **P3A.6** | [[Wireless]] | `wireless.kicad_sch` | ⬜ |
| **P3A.7** | [[Storage]] | `storage.kicad_sch` | ⬜ |
| **P3A.8** | [[USB Debug]] | `usb_debug.kicad_sch` | ⬜ |
| **P3A.9** | [[Safety UI]] | `safety_ui.kicad_sch` | ⬜ |
| **P3A.10** | [[Battery Monitor]] | `battery_monitor.kicad_sch` | ⬜ |
| **P4A** | Phase A ERC + peer review → freeze | git tag `sch-A-v1` | ⬜ |
| **P5** | 모터 / Elmo 드라이버 확정 → 전압·전류 확정 | [[Decisions Log]] | 보류 |
| **P3B** | [[Motor Power]] schematic 추가 | `motor_power.kicad_sch` | 보류 |
| **P4B** | 통합 ERC → freeze | git tag `sch-v1` | ⬜ |
| **P6** | PCB 레이아웃 (50×50mm 6-layer) | `*.kicad_pcb` | ⬜ |
| **P7** | DRC + gerber + 발주 | gerber zip | ⬜ |
| **P8** | 브링업 (Phase A USB 5V 단독 동작 가능) | 체크리스트 | ⬜ |

## 원칙

- Phase A 는 [[Modular Phase A-B Strategy|모터 결정 기다리지 않음]]
- 각 블록 sheet 는 ERC 단독 통과 후 통합
- 부품 발주는 P4A 이후 (스펙 freeze 됨)
