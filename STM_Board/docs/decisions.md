# Design Decisions Log

| 날짜 | 항목 | 결정 | 근거 |
|---|---|---|---|
| 2026-04-17 | 디렉토리 | `AR_Walker/STM_Board/` + `~/stm_board` 심링크 | git + 로컬 |
| 2026-04-17 | 모터 모델 | 보류 (Phase 5) — Elmo 가정 | 정밀 제어 |
| 2026-04-17 | 모터 인터페이스 | CANopen | Elmo 표준 |
| 2026-04-17 | 로드셀 ADC | ADS131M04 (24bit/32kSPS/4ch) | 앰프 강화 |
| 2026-04-17 | 로깅 | 온보드 microSD (SDMMC 4-bit) | 사용자 |
| 2026-04-17 | CAN 버스 | 1개 | 사용자 |
| 2026-04-17 | IMU | EBIMU 무선 (수신기 UART) | 사용자 |
| 2026-04-17 | EtherCAT | 사용 안 함 | 사용자 |
| 2026-04-17 | DMA | 고속 주변기기 전부 매핑 | 사용자 |
| 2026-04-17 | 무선 제어 | ESP32-C3-MINI-1U (IPEX 외부 안테나) | wearable |
| 2026-04-17 | Jetson 통신 | UART (DMA) + HW SYNC GPIO | 결정적·저지연 |
| 2026-04-17 | Link health watchdog | 무수신 시 motor enable 차단 | fail-safe |
| 2026-04-17 | Buck | TPS62933 (2.5MHz) | 컴팩트 |
| 2026-04-17 | 신호 커넥터 | JST-GH 1.25mm 통일 | 컴팩트 |
| 2026-04-17 | MCU | STM32H723VGT6 (LQFP100) | 외주 조립이라도 LQFP가 나음 |
| 2026-04-17 | Ethernet | 완전 제거 | 사용자 |
| 2026-04-17 | USB-C 디버그 | 포함 | DFU 가치 |
| 2026-04-17 | E-stop | 이중 안전 (SW + HW AND gate) | MCU 행 시도 차단 |
| 2026-04-17 | 모터 전원 보드 통과 | 확정 | 사용자 |
| 2026-04-17 | 모터 전류 | 15A+ 연속 / 30A 피크 (모터당) | 사용자 정보 |
| 2026-04-17 | DC 입력 커넥터 | XT60PW (30A 연속) | 모터 합 30A |
| 2026-04-17 | eFuse | LTC4368-1 + IPB180N06S4 N-MOSFET | OV/UV/RV/OCP/inrush 통합 |
| 2026-04-17 | 백업 퓨즈 | 30A automotive blade | 2차 안전망 |
| 2026-04-17 | 벌크 캡 | 470µF/63V Al-poly × 4 + 22µF X7R × 4 | 회생·inrush·dV/dt |
| 2026-04-17 | Common-mode choke | 60A 정격 | EMI |
| 2026-04-17 | **PCB Layer** | **6-layer 양면 실장** | 사용자 "layer↑로 사이즈↓ + 성능·안전" |
| 2026-04-17 | **보드 사이즈** | **50 × 50 mm** | 6L로 컴팩트 가능, wearable 적합 |
| 2026-04-17 | PCB 외층 동박 | 2 oz/ft² | 30A 트레이스 |
| 2026-04-17 | PCB 내층 동박 | 1 oz/ft² | 신호·전원 분산 |
| 2026-04-17 | 표면 처리 | ENIG | 정밀도·산화 방지 |
| 2026-04-17 | 스택업 | SIG/GND/SIG/PWR/GND/SIG | 모든 신호 GND 인접 |
| 2026-04-17 | 트레이스/간격 | 4mil/4mil 최소 | 6L 컴팩트 라우팅 |
| 2026-04-17 | Via-in-pad | 사용 (epoxy fill + cap plating) | 컴팩트 + BGA-like 부품 |
| 2026-04-17 | 모터 출력 커넥터 | **XT60 × 2 (안전 마진)** | 사이즈 절약된 만큼 안전성 ↑ |

## 보류 항목

- 6L (50×50) vs 8L (45×45) — 사용자 확정 필요
- CubeMX 칩 — 다른 H7로 세팅했는지
- INA226 입력 전류 모니터 포함 여부 (~30mm²)
- 모터 모델·정확한 전압·max 전류 (Phase 5)
