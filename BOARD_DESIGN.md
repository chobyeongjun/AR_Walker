# 새 보드 설계 → 제작 → 검증 전체 파이프라인

> 기준 시스템: AR_Walker (Teensy 4.1 기반) → STM32 커스텀 보드

---

## 현재 시스템에서 추출한 하드웨어 요구사항

| 기능 | 현재 (Teensy) | 새 보드 목표 |
|------|--------------|-------------|
| 제어 루프 | 500Hz | 500Hz 이상 |
| CAN 채널 | 1채널 (FlexCAN) | 2채널 (좌/우 독립) |
| 로드셀 ADC | 2채널 12bit (A16, A6) | **4채널** 24bit (관절별 독립) |
| IMU 통신 | UART Serial4 (Pin 16) | UART 동일 유지 |
| 배터리 모니터 | INA219 (I2C 0x40) | 동일 |
| 모터 STOP | GPIO Pin 9 (Active HIGH) | 동일 |
| BLE/통신 MCU | Nano 33 BLE 별도 | UART 헤더로 연결 |
| Debug | USB Serial | SWD + USB CDC |

---

## Phase 1: 블록 다이어그램 & 스펙 확정

### 시스템 블록

```
배터리 (24V LiPo)
    │
    ├─ AK 모터 x4 ← CAN-FD (1Mbps) ────────┐
    │                                        │
    └─ 보드 전원 (24V→5V→3.3V)              │
           │                          STM32H743 (LQFP100)
           │                                │
    ┌──────┴──────────────────────────────── ┤
    │  INA219 (전류/전력 모니터)             │
    │  로드셀 x4 ── INA333 x4 ── ADS1234 ──┤ (SPI)
    │  IMU ────────────────────── UART1 ────┤
    │  Jetson/PC ─────────────── UART2 ────┤
    │  USB CDC ─────────────────────────── ┤
    │  SWD 디버그 ──────────────────────── ┤
    └───────────────────────────────────────┘
```

### MCU 선택: STM32H743VIT6

| 항목 | 스펙 |
|------|------|
| 코어 | Cortex-M7, 480MHz, DP-FPU |
| 패키지 | **LQFP100** (손납땜 가능) |
| Flash | 2MB |
| RAM | 1MB (TCM 포함) |
| CAN-FD | **2채널** (FDCAN1, FDCAN2) |
| ADC | 3채널 16bit |
| UART | 4개 이상 |
| SPI | 6개 |
| I2C | 4개 |
| USB | Full-Speed |
| 가격 | ~$10 (LCSC 기준) |

---

## Phase 2: 컴포넌트 선정 (BOM)

### 핵심 IC 목록

#### MCU
| 부품 | 파트넘버 | 역할 | 비고 |
|------|---------|------|------|
| MCU | STM32H743VIT6 | 메인 컨트롤러 | LQFP100 |
| Crystal | 25MHz ±10ppm | HSE 클럭 | 부하 캐패시터 계산 필요 |

#### CAN 트랜시버
| 부품 | 파트넘버 | 역할 | 비고 |
|------|---------|------|------|
| CAN TR #1 | TJA1051T/3J | FDCAN1 → 좌측 모터 | 5V 동작, 3.3V 로직 호환 |
| CAN TR #2 | TJA1051T/3J | FDCAN2 → 우측 모터 | 동일 |
| 종단저항 | 120Ω (0402) | CAN 종단 | 보드 끝단에 위치 |
| TVS | PESD1CAN | ESD 보호 | CAN_H, CAN_L 각각 |

> **왜 TJA1051인가**: 3.3V 로직 호환, 차동 출력 ±5V, CAN-FD 5Mbps 지원, 저렴

#### 로드셀 ADC (핵심)
| 부품 | 파트넘버 | 역할 | 비고 |
|------|---------|------|------|
| 계측 앰프 x4 | INA333AIDGK | 로드셀 신호 증폭 | SOT-23-8, Gain = 1+100kΩ/R_G |
| 로드셀 ADC | ADS1234IPW | 24bit, 4채널 동시 | SPI 인터페이스 |
| 기준전압 | REF3325AIDBZR | 2.5V 정밀 기준 | ADS1234 VREF 입력 |

> **왜 ADS1234인가**: 24bit → 로드셀 0.1N 분해능, 4채널 동시 샘플링,  
> HX711보다 훨씬 우수한 노이즈 성능 (노이즈: 40nV rms)

**INA333 게인 설정 (예시):**
```
로드셀 출력: 보통 1~3 mV/V (여기서 V=5V → 5~15mV full-scale)
ADS1234 입력 범위: ±VREF = ±2.5V

필요 게인: 2500mV / 15mV ≈ 167 → R_G = 100kΩ / (167-1) ≈ 602Ω → 604Ω 표준 사용
→ 실제 게인: 100k/604 + 1 = 166.6 (fine)
```

#### 전원 관리
| 부품 | 파트넘버 | 역할 | 비고 |
|------|---------|------|------|
| 벅 컨버터 | AP62200WU | 24V → 5V, 2A | SOT-23-6, 효율 90%↑ |
| LDO | AMS1117-3.3 | 5V → 3.3V, 800mA | MCU/디지털 전원 |
| LDO (아날로그) | ADP3338AKCZ-3.3 | 5V → 3.3V (아날로그 전용) | 로드셀/ADC 분리 전원 |
| 역전압 보호 | P-MOS + 저항 | 배터리 역접속 방지 | |
| 전류 모니터 | INA219AIDCNR | 배터리 전류/전압 | I2C, 기존 코드 호환 |
| TVS (전원) | SMBJ26A | 24V 과전압 보호 | |

> **아날로그/디지털 전원 분리**: 로드셀 신호가 μV 수준 → 디지털 노이즈 유입 차단 필수

#### 기타
| 부품 | 파트넘버 | 역할 |
|------|---------|------|
| LED RGB | KT-0603RGB | 상태 표시 (기존 코드 호환) |
| 커넥터 CAN | JST-GH 4핀 | 모터 CAN 케이블 |
| 커넥터 로드셀 | Molex 51021-4핀 | 로드셀 4핀 (VCC/GND/S+/S-) |
| 커넥터 SWD | Tag-Connect TC2030 | 디버그 (보드 공간 절약) |
| USB | USB Type-C | 전원/CDC 겸용 |
| EEPROM | M24C04-RMN6TP | 캘리브레이션 값 저장 |

---

## Phase 3: 회로 설계 (KiCad)

### KiCad 설치 및 프로젝트 구조

```bash
# KiCad 8.0 설치 (macOS)
brew install --cask kicad

# 프로젝트 구조
stm_board/
├── kicad/
│   ├── walker_board.kicad_pro
│   ├── walker_board.kicad_sch    ← 회로도
│   ├── walker_board.kicad_pcb    ← PCB
│   ├── symbols/                  ← 커스텀 심볼
│   ├── footprints/               ← 커스텀 풋프린트
│   └── fabrication/              ← Gerber 출력
```

### 회로도 작성 순서 (시트 분할)

```
Sheet 1: MCU Core
  - STM32H743 + 디커플링 캐패시터 전체
  - 리셋 회로 (RC + 스위치)
  - 부트 모드 선택 (BOOT0 핀)
  - SWD 커넥터
  - HSE 크리스탈 회로

Sheet 2: Power
  - 입력 보호 (역전압, 과전압)
  - 24V → 5V 벅 컨버터
  - 5V → 3.3V LDO (디지털)
  - 5V → 3.3V LDO (아날로그, 분리)
  - INA219 배터리 모니터
  - 전원 LED 및 필터링

Sheet 3: CAN Interface
  - FDCAN1 + TJA1051 (좌측)
  - FDCAN2 + TJA1051 (우측)
  - 종단 저항
  - ESD 보호
  - JST-GH 커넥터 x4 (모터별)

Sheet 4: Loadcell Signal Chain
  - 로드셀 커넥터 x4
  - INA333 x4 (게인 설정 저항 포함)
  - ADS1234 (SPI 연결)
  - 기준전압 REF3325
  - 아날로그 전원 필터링
  - EMI 필터 (페라이트 비드)

Sheet 5: Peripherals
  - IMU UART 헤더
  - Jetson UART 헤더
  - USB Type-C + 보호 회로
  - RGB LED
  - 모터 STOP GPIO (기존 Active HIGH 유지)
  - EEPROM I2C
  - 확장 헤더 (GPIO, SPI 여유분)
```

### MCU 핀 할당 (STM32H743VIT6)

| 기능 | STM32 핀 | 비고 |
|------|---------|------|
| FDCAN1_TX | PD1 | 좌측 CAN |
| FDCAN1_RX | PD0 | 좌측 CAN |
| FDCAN2_TX | PB13 | 우측 CAN |
| FDCAN2_RX | PB12 | 우측 CAN |
| ADS1234_SCK | PA5 (SPI1) | |
| ADS1234_MISO | PA6 (SPI1) | |
| ADS1234_MOSI | PA7 (SPI1) | |
| ADS1234_CS | PA4 | |
| ADS1234_DRDY | PC4 | 인터럽트 입력 |
| IMU_UART_RX | PB7 (UART1_RX) | |
| JETSON_UART_TX | PD5 (UART2_TX) | |
| JETSON_UART_RX | PD6 (UART2_RX) | |
| INA219_SCL | PB8 (I2C1) | |
| INA219_SDA | PB9 (I2C1) | |
| USB_DM | PA11 | |
| USB_DP | PA12 | |
| MOTOR_STOP | PC13 | Active HIGH |
| LED_R | PB0 (TIM3_CH3) | PWM |
| LED_G | PB1 (TIM3_CH4) | PWM |
| LED_B | PE9 (TIM1_CH1) | PWM |
| SWDIO | PA13 | |
| SWDCLK | PA14 | |

---

## Phase 4: PCB 레이아웃

### 레이어 구성 (4-layer 필수)

```
Layer 1 (Top Copper):    신호 + 부품
Layer 2 (GND Plane):     디지털 GND 전면
Layer 3 (Power Plane):   3.3V / 5V 분리 영역
Layer 4 (Bottom Copper): 신호 + 보강
```

> **왜 4-layer**: 아날로그(로드셀) + 디지털(CAN, MCU) 혼재 → GND 플레인 분리 필수

### 레이아웃 규칙

#### 1. 전원 섹션
```
배터리 커넥터 → 역전압 보호 → 벅 컨버터 → LDO
모두 한쪽 코너에 몰아서 배치
입력 캐패시터: 컨버터 핀에서 2mm 이내
```

#### 2. 아날로그 섹션 (로드셀)
```
보드를 대각선으로 나눠서 아날로그 영역 격리
로드셀 커넥터 → INA333 → ADS1234 → MCU SPI
→ 이 경로는 디지털 신호 배선이 아래 층에서 교차하지 않도록

AGND와 DGND:
  - Layer 2는 AGND (아날로그 GND)
  - Layer 3의 AGND/DGND 분리 (스타 포인트에서만 연결)
```

#### 3. CAN 버스
```
차동 쌍 (CAN_H, CAN_L) 규칙:
- 길이 매칭: ±0.5mm
- 간격: 0.2mm
- 임피던스 100Ω 차동 (Layer 1/4: 55Ω → 차동 110Ω)
- Ground 가드링 추천
- TVS 다이오드 → 커넥터 바로 옆
```

#### 4. MCU 디커플링
```
STM32H743 전원핀마다:
- VDD 핀: 100nF (0402) → 핀에서 최대 0.5mm 이내
- VDDA 핀: 1μF + 100nF 병렬
- VCAP 핀: 2.2μF (STM32 내부 LDO용)
총 캐패시터 수: ~30개 이상 (H743 기준)
```

### 보드 사이즈 제안

```
100mm × 80mm (명함 2배 크기)
- JLCPCB 기준 최저가 구간
- 외골격 장착 고려 시 두께: 1.6mm
- 마운팅 홀: M3 x 4개 (코너)
```

---

## Phase 5: 전기전자 검증 (설계 완료 후)

### 5-1. 전원 계산

```
소비 전류 추정:
  STM32H743 @ 480MHz:     ~200mA (max)
  INA333 x4:              ~0.4mA
  ADS1234:                ~2mA
  TJA1051 x2:             ~70mA (활성)
  INA219:                 ~1mA
  LED:                    ~10mA
  여유분:                  ~20%
  ────────────────────────────────
  3.3V 레일 합계:          ~350mA (3.3V × 350mA = 1.15W)
  5V 레일 합계:            ~150mA (TJA1051 포함)
  
AP62200 (24V→5V) 필요 출력: 5V × (350/η + 150) ≈ 5V × 550mA
→ AP62200 (2A 정격) 충분, 여유 75%
```

### 5-2. 로드셀 신호 체인 검증

```
로드셀 출력 계산:
  감도: 2mV/V (일반적), 여기 VCC = 5V
  → full-scale 출력: 2mV/V × 5V = 10mV

INA333 출력:
  게인 = 1 + 100kΩ/604Ω = 166.6
  → 10mV × 166.6 = 1.666V (ADS1234 입력 범위 ±2.5V 내 ✓)

ADS1234 분해능:
  2^24 = 16,777,216 counts
  입력 범위 ±2.5V → 5V / 2^24 = 298nV/count
  → 로드셀 환산: 298nV / (166.6 × 5V/1N*) ≈ 수십 mN 분해능
  (* 로드셀 감도에 따라 다름)

SNR 확인:
  ADS1234 노이즈: ~40nV rms (데이터시트)
  INA333 노이즈: 50nV/√Hz × √500Hz ≈ 1.1μV
  → 신호 대비 노이즈: 1666000μV / 1.1μV ≈ 63dB ✓
```

### 5-3. CAN 버스 타이밍 계산

```
STM32H743 FDCAN 클럭 설정:
  HSE: 25MHz → PLL → FDCAN 클럭: 80MHz
  
  목표: 1Mbps (AK 모터 기존 설정 유지)
  
  Prescaler = 4 → Time Quantum = 1/(80MHz/4) = 50ns
  Bit time = 1/1Mbps = 1000ns = 20 TQ
  
  권장 분배:
    Sync_Seg: 1 TQ (고정)
    Prop + Phase1: 13 TQ (샘플 포인트 75%)
    Phase2: 6 TQ
  → 샘플 포인트: (1+13)/20 = 70% ✓ (권장 70~80%)
```

### 5-4. KiCad DRC 체크리스트

```
전기 규칙 (ERC):
  □ 연결되지 않은 핀 없음
  □ 전원 핀 방향 올바름
  □ 버스 연결 레이블 일치
  □ 레퍼런스 중복 없음

PCB 규칙 (DRC):
  □ 최소 트레이스 폭: 0.15mm (신호), 0.5mm (전원)
  □ 최소 간격: 0.15mm
  □ 차동 쌍 길이 매칭 허용 오차 ±0.5mm
  □ 쿨리어런스 위반 없음
  □ 언루티드 넷 없음
  □ 마운팅 홀 GND 연결 확인
  □ 실크 겹침 없음
```

### 5-5. Gerber 생성 및 발주

```
JLCPCB 발주 설정:
  레이어: 4
  크기: 100mm × 80mm
  두께: 1.6mm
  표면처리: ENIG (금도금, 로드셀 커넥터 신뢰성)
  최소 트레이스: 6/6mil
  수량: 5pcs (첫 프로토타입)
  예상 가격: ~$20-30
  
KiCad Gerber 출력 파일:
  - F_Cu.gbr (Top Layer)
  - B_Cu.gbr (Bottom Layer)
  - In1_Cu.gbr (GND Plane)
  - In2_Cu.gbr (Power Plane)
  - F_Mask.gbr / B_Mask.gbr
  - F_Silkscreen.gbr / B_Silkscreen.gbr
  - Edge_Cuts.gbr (보드 외곽)
  - drill.drl (드릴 파일)
```

---

## Phase 6: 보드 Bring-up 순서

### 6-1. 전원 검증 (부품 없이 전원부터)

```
1. 전원부만 납땜
2. 역전압 보호 테스트 (배터리 반대로 꽂기)
3. 24V 인가 → 5V 출력 확인 (멀티미터)
4. 5V → 3.3V 확인
5. 전류 소비: 아무 IC 없는 상태에서 ~0mA 확인
```

### 6-2. MCU 검증

```
1. STM32H743 납땜
2. SWD로 연결 (ST-Link V3)
3. LED 점멸 테스트 코드 플래시
4. UART2로 "Hello" 출력 확인 (Jetson UART)
```

### 6-3. CAN 검증

```
1. TJA1051 납땜
2. STM32 FDCAN 루프백 테스트 (자기 자신 수신)
3. AK 모터 연결 → Enable/Disable 명령
4. SET_POS 단순 위치 명령
```

### 6-4. 로드셀 검증

```
1. INA333 + ADS1234 납땜
2. SPI 통신 확인 (ID 레지스터 읽기)
3. 로드셀 오프셋 영점 조정
4. 손으로 눌러서 값 변화 확인
5. 실제 힘 vs 로드셀 값 캘리브레이션
```

---

## 전체 일정

| 주차 | 작업 | 결과물 |
|------|------|--------|
| 1주 | 컴포넌트 선정 확정, BOM 작성 | BOM.xlsx |
| 2주 | KiCad 스키매틱 (Sheet 1-3) | .kicad_sch |
| 3주 | KiCad 스키매틱 (Sheet 4-5) + ERC | 오류 0개 |
| 4주 | 풋프린트 할당 + PCB 레이아웃 | .kicad_pcb |
| 5주 | 전기 검증 + DRC 통과 | DRC 오류 0개 |
| 6주 | Gerber 생성 + JLCPCB 발주 | 주문 완료 |
| 7-8주 | 부품 조달 + 납땜 | 보드 완성 |
| 9주 | Bring-up Phase 1-4 | 동작 확인 |
| 10주 | AR_Walker 펌웨어 STM32 포팅 | 500Hz 루프 동작 |

---

## 즉시 할 것 (이번 주)

- [ ] KiCad 8.0 설치
- [ ] STM32H743VIT6 데이터시트 다운로드 (핀맵 인쇄)
- [ ] LCSC에서 위 부품 재고/가격 확인
- [ ] BOM.xlsx 작성 시작
- [ ] 개발용 Nucleo-H743ZI 주문 (PCB 전에 펌웨어 먼저 검증)
