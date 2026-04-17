# Design Decisions Log

각 결정은 `날짜 — 항목 — 결정 — 근거` 포맷으로 누적.

| 날짜 | 항목 | 결정 | 근거 / 대안 |
|---|---|---|---|
| 2026-04-17 | 디렉토리 구조 | `AR_Walker/STM_Board/` 내에 모든 산출물 + `~/stm_board` 심링크 | git 추적 + 로컬 편의성 둘 다 |
| 2026-04-17 | MCU 1순위 | STM32G474RE (사용자 확인 대기) | HRTIM + CAN-FD, 모터제어 특화 |
| 2026-04-17 | 모터 결정 | **보류 (Phase 5)** | 보드는 CAN-FD + 전원분리로 일반화 |
| 2026-04-17 | IMU | BNO055 1차 채택 | 기존 Walker_Main 코드 재활용 가능 |

## 보류 항목

- 모터 모델 (CubeMars vs Maxon vs T-Motor)
- 공급 전압 24/36/48V
- Jetson 1순위 인터페이스 (UART/USB/Ethernet)
- SD 로깅 포함 여부
