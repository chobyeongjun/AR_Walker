---
aliases: [BOM]
tags: [moc, bom]
---

# BOM MOC

## 최신 BOM

- **[[BOM_v3]]** — CSV, v3.5 reconciliation 반영
  - MCU H743 / ISO1050 / 보호 아키텍처 완전 / 시스템 모듈 포함

## 히스토리

| 버전 | 날짜 | 주요 변경 |
|---|---|---|
| v0 | 2026-04-17 | 초기 scaffold |
| v1 | 2026-04-17 | ADS131M04, ESP32, H7 전환 |
| v2 | 2026-04-20 | Obsidian 리팩토링 + Phase A/B 분리 |
| **v3** | 2026-04-20 | **RECONCILIATION: H723→H743, TCAN1462→ISO1050, 보호 아키텍처, 2× INA228, 시스템 스펙** |

## 구조

CSV 컬럼: `block, ref, part, package, qty, source, notes, phase, decided`

- `phase`: A (모터 무관) / B (모터 의존)
- `decided`: yes / no / tbd

블록 prefix:
- `mcu` — STM32H743 코어
- `logic_power` — 5V/3.3V 전원
- `loadcell` — ADS131M04 체인
- `encoder` — AS5048A × 2
- `comms_can` — ISO1050 격리 체인
- `comms_uart` — Jetson/EBIMU/SYNC 커넥터
- `wireless` — ESP32-C3
- `usb` — USB-C 디버그
- `storage` — microSD
- `ui` — LED / 버튼
- `safety` — E-stop + AND gate
- `battery` — INA228 × 2
- `motor_power` (Phase B) — DC 입력·역전압·인러시·버스·브레이크
- `motor_power_local` (Phase B) — 모터당 로컬 보호 × 4
- `grounding` — star ground 페라이트
- `system` — 외부 모듈 (Jetson/카메라/IMU)

## 발주 전 체크

- [ ] 각 부품 LCSC/Mouser/Digikey 재고·단가 확인
- [ ] 대체 부품 (second source) 기록
- [ ] KiCad 풋프린트 매칭 확인
- [ ] 패시브 (R/C) 정리 (0402 통일 등)
- [ ] Phase B 모터 확정 후 `decided=tbd` 항목 갱신

## Phase B 이후 확정되는 것들

- 커넥터 XT60 vs XT90 vs XT30 (모터 전류 확정 후)
- Buck 컨버터 AP63205WU-7 (24V) vs TPS62933 (48V 대응)
- 퓨즈 정격 15A/30A/40A
- 배터리 V 의존 부품 (48V 면 SMAJ56CA, P6KE68CA 등)
