# Exosuit STM32 보드 설계 플랜

> Cable-Driven Exosuit 제어용 커스텀 STM32 보드. KiCad로 처음부터 설계.
> 기존 `Walker_Main` (Teensy + CubeMars/Maxon CAN, BNO055 IMU, 로드셀) 구조를 STM32로 이식·확장.

---

## 0. 목표 / 비기능 요구사항

| 항목 | 결정 / 제약 |
|---|---|
| 용도 | Exosuit (Cable-driven) 정밀 토크 제어 |
| 액추에이터 | 2x BLDC 모터 (모델 선정은 **Phase 5에서 확정** — 보드는 인터페이스만 일반화) |
| 호스트 | NVIDIA Jetson (Orin Nano 가정) — 고수준 제어 / 학습 추론 |
| 제어 루프 | ≥1 kHz current/torque loop, ≥200 Hz outer loop |
| 안전 | E-stop, 모터 디스에이블 핀, 워치독, 과전류·과전압 보호 |
| 로깅 | SD 카드 또는 Jetson 측 로깅 (선택) |
| 폼팩터 | 가능한 작게 (착용형). 1차 시제품은 4-layer 기준 |

---

## 1. 시스템 블록 다이어그램 (텍스트)

```
                ┌─────────────────────────────────────────────┐
                │              Jetson Orin (host)            │
                └──────────────┬──────────────┬──────────────┘
                               │ USB / UART  │ (옵션) Ethernet
                ┌──────────────▼─────────────▼──────────────┐
                │                STM32 MCU                    │
                │  (G4: 모터제어 강점 / H7: 성능여유)          │
                │                                              │
                │  CAN-FD (모터)   UART (Jetson, IMU)         │
                │  SPI  (SD/IMU 옵션)  I2C (보조)             │
                │  ADC  (로드셀, 전류, 온도)                   │
                │  Timer/PWM (모터 직결 옵션, LED, sync)       │
                └──┬──────┬──────┬──────┬──────┬──────┬──────┘
                   │      │      │      │      │      │
              ┌────▼─┐ ┌──▼──┐ ┌─▼──┐ ┌─▼──┐ ┌▼──┐ ┌─▼──┐
              │ CAN  │ │IMU  │ │Load│ │ENC │ │LED│ │SD  │
              │ XCVR │ │BNO055│ │HX711│ │AS5x│ │RGB│ │card│
              └──┬───┘ └─────┘ └────┘ └────┘ └───┘ └────┘
                 │ (CAN bus)
            ┌────▼────┐  ┌────────┐
            │ Motor 1 │  │ Motor 2│   ← Phase 5에 모델 확정
            └─────────┘  └────────┘

  Power: 24V (or 36/48V) ─► 5V buck ─► 3.3V LDO ─► MCU/센서
                          └► 모터로 직결 (보호회로 경유)
```

---

## 2. MCU 선정 (1순위 후보)

| 후보 | 장점 | 단점 |
|---|---|---|
| **STM32G474RE** (LQFP64) | 모터 제어 특화(HRTIM), CAN-FD x3, FPU, 가성비, 주변기기 풍부 | 200MHz vs H7 / RAM 128KB |
| STM32H723ZG (LQFP100) | 550MHz, RAM 충분, 미래 확장 | 패키지 큼, 발열·전력 |
| STM32F405RG | 실적 많음, 자료 풍부 | CAN-FD 없음, 구식 |

**권장: STM32G474RE 로 시작.** Cable-driven exosuit 토크 제어에 충분, CAN-FD로 차세대 BLDC 드라이버까지 커버.
(추후 H7로 갈아끼울 여지 — 핀맵을 G4 기준으로 잡되, 풋프린트 호환은 어려우니 일단 G4 확정)

> ❓ 결정 필요: STM32CubeMX에서 이미 어떤 패밀리로 세팅해뒀는지 알려줘. 그 칩으로 맞출게.

---

## 3. 보드 부품 리스트 (블록별)

### 3-1. 전원 (Power tree)
- [ ] **DC 입력 단자** : XT30 또는 4-pin Molex, 24V 입력 가정 (모터 전압에 맞게 조정)
- [ ] **역전압 보호** : P-MOSFET ideal diode (e.g. SI4435 + 다이오드)
- [ ] **TVS 다이오드** : SMBJ33A 등 입력단
- [ ] **퓨즈** : Polyfuse or replaceable (5–10A)
- [ ] **24V → 5V Buck** : TPS54360 / LMR14030 (≥3A 여유)
- [ ] **5V → 3.3V LDO** : AMS1117-3.3 또는 MIC5219 (저노이즈)
- [ ] **벌크 캡** : 100µF + 47µF + 10µF + 100nF
- [ ] **전원 LED** : 3.3V rail 표시
- [ ] **(옵션) 별도 아날로그 3.3V** : ADC 정밀도 위해 ferrite + LDO 분리

### 3-2. MCU 코어
- [ ] **STM32G474RE** (LQFP64)
- [ ] **8 MHz HSE 크리스털** + 22pF cap x2
- [ ] **32.768 kHz LSE** (RTC 쓸지 결정 후)
- [ ] **VDD/VDDA 디커플링** : 100nF x N + 4.7µF
- [ ] **NRST 풀업 + 100nF**
- [ ] **BOOT0 풀다운 + 점퍼 (DFU 부트로더 진입용)**
- [ ] **SWD 헤더 (4 또는 5pin: SWDIO, SWCLK, GND, 3V3, NRST)**

### 3-3. 모터 인터페이스 (모델은 미확정 — 인터페이스만 표준화)
- [ ] **CAN-FD 트랜시버 x1 (or 2)** : TCAN1462 / MCP2562FD
- [ ] **CAN 버스 헤더** (JST-XH 4pin: CAN_H, CAN_L, GND, +24V 분기 옵션)
- [ ] **120Ω 종단 + 점퍼**
- [ ] **모터 전원 차단 스위치** (logic-level MOSFET, MCU 제어)
- [ ] **(옵션) 모터당 Estop 라인** : opto-isolated input
- [ ] (예비) PWM x2 + DIR + ENABLE 헤더 — 직결 드라이버 가능성 대비

### 3-4. 센서
- [ ] **IMU**: BNO055 (UART 모드, 기존 코드 재활용) 또는 ICM-42688-P (SPI, 더 정확)
  - 결정 미루기: 일단 풋프린트 둘 다 옵션 두지 말고 **BNO055로 통일** (코드 호환)
- [ ] **로드셀 ADC** : HX711 x2 (각 케이블당) — I2C/SPI 아닌 자체 프로토콜, GPIO로
  - 또는 ADS1232 (더 빠른 샘플링 필요 시)
- [ ] **케이블 위치 인코더** : AS5048A (SPI 자기식) x2 — pulley 회전각
- [ ] **모터 전류 센스** : INA240 or 모터 드라이버 내장 사용
- [ ] **온도 센서** : NTC x2 (모터 케이스), MCU 내장 온도

### 3-5. Jetson 통신
- [ ] **UART (TTL 3.3V)** : Jetson GPIO 헤더 직결, 4-pin JST (TX/RX/GND/3V3)
- [ ] **(옵션) USB Full-Speed** : MCU 내장 USB → Jetson USB 포트
- [ ] **(옵션) SPI follower** : 고속 텔레메트리 필요 시

### 3-6. 사용자 인터페이스
- [ ] **RGB 상태 LED** (PWM 3채널, 기존 코드 호환)
- [ ] **Sync LED** (IMU·외부 시스템 동기 펄스)
- [ ] **버튼 x2** (RESET + USER)
- [ ] **부저** (선택) — 에러 알림

### 3-7. 저장 / 로깅
- [ ] **microSD 슬롯 (SPI 또는 SDIO)** — `Walker_Main` 의 PiLogger처럼

### 3-8. 보호 / EMI
- [ ] CAN 라인 TVS (PESD1CAN)
- [ ] 모든 외부 헤더에 ferrite bead
- [ ] ESD 다이오드 USB D+/D-
- [ ] 정전기 보호 0.1µF 캡 입력단

---

## 4. 인터페이스 / 핀맵 초안 (STM32G474RE 기준)

| 기능 | 핀 (제안) | 비고 |
|---|---|---|
| CAN1-FD TX/RX | PB9 / PB8 | 모터 버스 |
| UART2 (Jetson) | PA2 / PA3 | 115200~1Mbps |
| UART4 (IMU BNO055) | PC10 / PC11 | 기존 코드 호환 |
| SPI1 (Encoder AS5048) | PA5/6/7 + CS PA4, PB0 | 모터 2개 각각 CS |
| SPI3 (SD card) | PB3/4/5 + CS PA15 | |
| ADC1 (Loadcell raw, 옵션) | PA0, PA1 | HX711 안 쓸 경우 |
| HX711 SCK/DT x2 | PB10–PB13 | bit-bang |
| RGB LED | PA8/PA9/PA10 (TIM1) | PWM |
| Sync LED | PB14 | |
| Motor enable | PB15 | |
| User button | PC13 | |
| SWD | PA13 / PA14 | |
| BOOT0 | BOOT0 핀 | 점퍼 |

> 이 표는 1차 초안. CubeMX에서 충돌 검사 후 확정.

---

## 5. 단계별 진행 (Phase Plan)

| Phase | 내용 | 산출물 | 상태 |
|---|---|---|---|
| **P0** | 요구사항·블록다이어그램 확정 (이 문서) | `PLAN.md` | 🟡 진행중 |
| **P1** | 부품 후보 결정·BOM 초안 | `parts/BOM_v0.csv`, 데이터시트 수집 | ⬜ |
| **P2** | KiCad 프로젝트 생성, 심볼·풋프린트 라이브러리 정리 | `kicad/exo_stm32.kicad_pro` | ⬜ |
| **P3** | 블록별 schematic (Power / MCU / Comms / Sensor / Motor IF) | `*.kicad_sch` | ⬜ |
| **P4** | ERC, 시뮬·검토 → schematic v1 freeze | git tag `sch-v1` | ⬜ |
| **P5** | **모터 모델 확정 → 모터 IF 블록 재검토** | 결정 문서 | ⬜ (보류) |
| **P6** | PCB 레이아웃 (4-layer, 부품 배치 → 라우팅) | `*.kicad_pcb` | ⬜ |
| **P7** | DRC, 거버 출력, 시제품 발주 | gerber zip | ⬜ |
| **P8** | 조립·브링업 (전원→MCU→통신→센서→모터) | 브링업 체크리스트 | ⬜ |

**원칙:** 모터 결정 전에도 P0–P4 + P6의 *모터 외* 부분까지 진행 가능.
모터 IF는 "CAN-FD + Estop + 전원 분리"로 일반화해두면 80% 호환됨.

---

## 6. 즉시 결정 필요 (사용자 확인 사항)

1. **STM32 칩 패밀리** : CubeMX에 이미 어떤 칩으로 세팅돼 있어? (G4 / H7 / F4?)
2. **공급 전압** : 모터 시스템 24V / 36V / 48V 중 무엇 가정?
3. **Jetson 통신 1순위** : UART vs USB vs Ethernet?
4. **로깅** : 보드에 SD카드 넣을지, Jetson에서 받아 저장할지?
5. **IMU** : BNO055 (기존 호환) 그대로 갈지, 더 좋은 것(ICM-42688)으로 바꿀지?
6. **CAN-FD 모터 1버스 vs 2버스** : 격리·중복성 고려 시 2버스가 안전

---

## 7. 참고 자료 / 기존 자산

- `Walker_Main/Hardware/PCB/CAN_PCB/` — 기존 CAN 보드 zip + guide docx
- `Walker_Main/Hardware/PCB/Maxon_PCB/` — Maxon용 보드 노트
- `Main/main/src/Board.h` — Teensy 4.1 핀맵 (이식 시 참고)
- `Main/main/src/CAN.h`, `Motor.h`, `IMU.h` — 드라이버 인터페이스
- 외부 레퍼런스 (조사 예정):
  - OpenExo (NAU) — schematic 공개
  - ODrive v3.6 — 모터 전원/CAN 보호회로
  - MJBots Moteus — STM32G4 + CAN-FD 단일 보드

---

## 8. 디렉토리 구조

```
STM_Board/                        ← AR_Walker 레포 내 (git tracked)
├── PLAN.md                       ← 이 문서
├── docs/                         ← 결정 기록, 다이어그램
├── kicad/                        ← KiCad 프로젝트 일체
├── parts/                        ← BOM, 데이터시트 링크/파일
├── refs/                         ← 참고 보드 자료
└── schematic_blocks/             ← 블록별 sub-sheet 작업 메모

~/stm_board                       ← 로컬 거울 (선택 — 심볼릭 링크 권장)
```

> 권장: `ln -s ~/AR_Walker/STM_Board ~/stm_board` — git 추적과 로컬 폴더 둘 다 잡힘.
