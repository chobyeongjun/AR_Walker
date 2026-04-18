# Design Decisions Log

각 결정은 `날짜 — 항목 — 결정 — 근거` 포맷으로 누적.

| 날짜 | 항목 | 결정 | 근거 / 대안 |
|---|---|---|---|
| 2026-04-17 | 디렉토리 구조 | `AR_Walker/STM_Board/` + `~/stm_board` 심링크 | git 추적 + 로컬 편의성 |
| 2026-04-17 | MCU 1순위 (v1) | STM32G474RE | 모터 제어 특화 — **v2에서 H7로 변경** |
| 2026-04-17 | IMU | EBIMU 무선 (E2BOX) → 수신기 UART로 MCU 직결 | 사용자 보유 장비 활용, 지연 최소화 |
| 2026-04-17 | 모터 드라이버 | **Elmo (모델 미정)** — CANopen 가정 | Phase 5 모터와 함께 확정 |
| 2026-04-17 | 모터 모델·전압 | **보류 (Phase 5)** | 정밀 제어 위해 후순위 결정 |
| 2026-04-17 | MCU 변경 (v2) | **STM32H723ZGT6** | 1kHz 제어 + SDIO + USB HS + 고속 ADC 동시 처리 여유 |
| 2026-04-17 | 로드셀 ADC | HX711 → **ADS131M04** (24bit/32kSPS/4ch 동시) | 사용자 요구 "앰프 강화" |
| 2026-04-17 | 로깅 | 온보드 microSD (SDMMC 4-bit) | 사용자 확정 |
| 2026-04-17 | CAN 버스 수 | 1개 | 사용자 확정 |
| 2026-04-17 | 온보드 IMU | 제거 | EBIMU 무선 사용 |
| 2026-04-17 | EtherCAT | **사용 안 함 (확정)** | 사용자 확정 |
| 2026-04-17 | DMA 활용 (v2.1) | **고속 주변기기 전부 DMA 매핑** (PLAN §4-9) | 사용자 요구 |
| 2026-04-17 | 무선 제어 (v2.1) | **온보드 ESP32-C3-MINI-1 (BLE 5.0 + WiFi)** | 사용자 요구 "실시간 무선 통제" |
| 2026-04-17 | Jetson 데이터 플로우 (v2.2) | **Jetson 카메라→포즈 다운링크 60~100Hz, ~50KB/s, 지연 <20ms** 명시 | 사용자 컨텍스트 |
| 2026-04-17 | Jetson 통신 1순위 재추천 (v2.2) | **고속 UART + 별도 HW SYNC GPIO** | 저지연·결정적·부품 최소, 데이터량 충분 |
| 2026-04-17 | HW Sync 라인 (v2.2) | MCU↔Jetson SYNC_OUT/SYNC_IN GPIO 헤더 추가 | 카메라 프레임 ↔ 제어 루프 동기 |
| 2026-04-17 | Link health watchdog | 무수신 시 motor enable 자동 차단 | 안전 fail-safe |

## 보류 항목

- **Jetson 통신 방식 사용자 최종 확정** (UART 추천 / USB / Ethernet)
- **USB-C 보조 포트** 항상 포함할지 (권장)
- **Ethernet PHY** DNF로 남길지 (사이즈 ~5% 증가)
- **ESP32 안테나** PCB 내장 vs 외부 IPEX
- 모터 모델 (Elmo Solo Whistle? Twitter? Bee?) - Phase 5
- 공급 전압 24/36/48V - 모터 후
- E-stop 외부 회로 유무
