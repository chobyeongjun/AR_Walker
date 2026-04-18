# Design Decisions Log

| 날짜 | 항목 | 결정 | 근거 |
|---|---|---|---|
| 2026-04-17 | 디렉토리 | `AR_Walker/STM_Board/` + `~/stm_board` 심링크 | git + 로컬 |
| 2026-04-17 | 모터 모델 | 보류 (Phase 5) — Elmo 가정 | |
| 2026-04-17 | 모터 인터페이스 | CANopen | Elmo 표준 |
| 2026-04-17 | 로드셀 ADC | ADS131M04 (24bit/32kSPS/4ch) | 앰프 강화 |
| 2026-04-17 | 로깅 | 온보드 microSD (SDMMC 4-bit) | |
| 2026-04-17 | CAN 버스 | 1개 | |
| 2026-04-17 | IMU | EBIMU 무선 (수신기 UART) | |
| 2026-04-17 | EtherCAT | 사용 안 함 | |
| 2026-04-17 | DMA | 고속 주변기기 전부 매핑 | |
| 2026-04-17 | 무선 제어 | ESP32-C3-MINI-1U (IPEX 외부 안테나) | |
| 2026-04-17 | Jetson 통신 | UART (DMA) + HW SYNC GPIO | |
| 2026-04-17 | Link health watchdog | 무수신 시 motor enable 차단 | |
| 2026-04-17 | Buck | TPS62933 (2.5MHz, 60V 입력) | 24/36/48V 모두 커버 |
| 2026-04-17 | 신호 커넥터 | JST-GH 1.25mm 통일 | |
| 2026-04-17 | MCU | STM32H723VGT6 (LQFP100) | |
| 2026-04-17 | Ethernet | 완전 제거 | |
| 2026-04-17 | USB-C 디버그 | 포함 | |
| 2026-04-17 | E-stop | 이중 안전 (SW + HW AND gate) | |
| 2026-04-17 | 모터 전원 보드 통과 | 확정 | |
| 2026-04-17 | 모터 전류 | 15A+ 연속 / 30A 피크 | 사용자 |
| 2026-04-17 | DC 입력 커넥터 | XT60PW (30A) | Phase B에서 모터 후 재확정 |
| 2026-04-17 | eFuse | LTC4368-1 + IPB180N06S4 | Phase B에서 MOSFET 사이즈 재확정 |
| 2026-04-17 | 백업 퓨즈 | 30A automotive blade | Phase B |
| 2026-04-17 | 벌크 캡 | 470µF Al-poly × 4 + 22µF X7R × 4 | Phase B (전압 정격은 모터 후) |
| 2026-04-17 | Common-mode choke | 60A 정격 | Phase B |
| 2026-04-17 | PCB Layer | 6-layer 양면 실장 | layer↑로 사이즈↓ |
| 2026-04-17 | 보드 사이즈 | 50 × 50 mm | wearable |
| 2026-04-17 | 외층/내층 동박 | 2 oz / 1 oz | 30A 트레이스 |
| 2026-04-17 | 표면 처리 | ENIG | |
| 2026-04-17 | 스택업 | SIG/GND/SIG/PWR/GND/SIG | |
| 2026-04-17 | 모터 출력 커넥터 | XT60 × 2 | Phase B |
| 2026-04-17 | 배터리 모니터링 | INA228 + 1mΩ shunt + NTC | "배터리 잔량 확인" |
| 2026-04-17 | SoC 알고리즘 | OCV + 쿨롱 카운팅 | 펌웨어 |
| 2026-04-17 | **모듈러 전략 (v2.9)** | **Phase A (로직, 지금 freeze) + Phase B (모터 의존, 보류)** | 사용자 "앰프 등 미리 만들기" |
| 2026-04-17 | Phase A schematic 단위 | hierarchical sheet 10개 (loadcell 우선) | |
| 2026-04-17 | KiCad 프로젝트 구조 | hierarchical sheets per block | 모듈러 |
| 2026-04-17 | 1차 PCB 통합 vs 분리 | 단일 보드, 분리 옵션은 P6에서 재검토 | 우선 단순 |

## 보류 항목

- 6L (50×50) vs 8L (45×45) — PCB 단계
- CubeMX 칩 H723VGT6 진행 OK 여부 — **지금 답변 필요**
- 모터 모델·전압·max 전류 (Phase B 시작 전)
- 단일보드 vs 로직/파워 분리 — PCB 단계
- 배터리 종류 (Li-ion 셀 수, 용량) — SoC 캘리브레이션용
