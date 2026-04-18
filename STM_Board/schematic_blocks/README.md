# Schematic Blocks (Hierarchical Sheets)

KiCad에서 hierarchical sheet 단위로 작업할 블록들. 각 `.md` 는 해당 sheet 작업 노트.

## Phase A (모터 무관 — 지금 진행)

| 파일 | 블록 | 우선순위 | 주요 부품 |
|---|---|---|---|
| `mcu_core.md` | MCU + 클럭 + 디커플링 + SWD | 1 | STM32H723VGT6 |
| `loadcell_amp.md` | **로드셀 앰프 (사용자 우선)** | **1** | ADS131M04 + REF5025 |
| `encoder.md` | 케이블 위치 인코더 | 2 | AS5048A × 2 |
| `comms.md` | CAN-FD + Jetson UART + SYNC + EBIMU | 2 | TCAN1462 |
| `wireless.md` | BLE/WiFi (ESP32) | 2 | ESP32-C3-MINI-1U |
| `storage.md` | microSD SDMMC | 3 | DM3AT |
| `usb_debug.md` | USB-C + ESD | 3 | TPD4S012 |
| `safety_ui.md` | E-stop + LED + 버튼 | 3 | 74LVC1G08 |
| `battery_monitor.md` | 배터리 SoC | 3 | INA228 |
| `logic_power.md` | 5V→3V3 LDO ×2 | 1 | TLV75533 |

## Phase B (모터 결정 후)

| 파일 | 블록 |
|---|---|
| `motor_power.md` | DC입력 + eFuse + 큰 캡 + 모터 출력 + buck input |

## 작업 순서

1. 각 블록마다 reference schematic을 `refs/datasheets/` 에 저장
2. 데이터시트 typical application 회로를 한 줄씩 차용
3. 핀맵 + 디커플링 + 보호회로 정리
4. KiCad sheet에 1:1 옮기기
