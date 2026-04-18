# Exosuit STM32 보드 설계 플랜 (v2)

> Cable-Driven Exosuit 정밀 제어용 커스텀 STM32 보드.
> v2 변경점: **MCU H7 전환, Jetson USB HS(결정 보류), EBIMU 무선 UART, 로드셀 ADS131M04, 모터 Elmo CANopen 가정 (EtherCAT 사용 안 함)**
> v2.1 추가: **DMA 적극 활용 매핑 명시 + 온보드 BLE/WiFi 모듈 (ESP32-C3) 추가 — 무선 실시간 통제 채널**

---

## 0. 목표 / 비기능 요구사항

| 항목 | 결정 |
|---|---|
| 용도 | Exosuit (Cable-driven) 정밀 토크 제어 |
| 액추에이터 | 2x BLDC + **Elmo 서보 드라이버 (모델 미확정, Phase 5)** |
| 호스트 | NVIDIA Jetson — **통신 방식 결정 대기** (USB HS / 일반 Ethernet TCP·UDP / 고속 UART 중 선택) |
| 제어 루프 | ≥1 kHz current/torque, ≥200 Hz outer |
| IMU | **EBIMU (E2BOX 무선)** — 수신기 UART → STM32 직결 (Jetson 경유 X, 지연 최소화) |
| 무선 제어 | **온보드 BLE 5.0 + WiFi (ESP32-C3-MINI-1)** — UART로 MCU 연결, 폰/태블릿/PC에서 실시간 통제 |
| DMA | **모든 고속 주변기기 DMA 매핑 필수** (§4-9 참조) — CPU 부하 최소화 |
| 로깅 | **온보드 microSD (SDMMC 4-bit)** |
| 안전 | E-stop, 모터 전원 게이트, IWDG 워치독, 과전류·과전압·과온 보호 |
| 폼팩터 | **컴팩트 필수 (wearable). 목표 ≤ 50×50mm 4-layer, 공격적 ≤ 45×40mm 6-layer** |

---

## 1. 시스템 블록 다이어그램

```
        ┌──────────────────────────────────┐
        │   Jetson Orin (host)             │
        └──────────┬─────────────┬─────────┘
                   │ USB HS      │ (옵션) Ethernet
       ┌───────────▼─────────────▼──────────┐
       │     STM32H723ZGT6 (550 MHz M7)     │
       │   1MB Flash / 564KB RAM / FPU      │
       │                                     │
       │  CAN-FD x1   FDCAN1                 │
       │  USB HS      OTG_HS (내장 PHY)      │
       │  RMII        ETH (옵션 LAN8742)     │
       │  SDMMC1      4-bit microSD          │
       │  SPI1        ADS131M04 (loadcell)  │
       │  SPI3        AS5048A x2 (encoder)  │
       │  UART (4ch)  EBIMU 수신기, debug,   │
       │              Jetson backup, spare   │
       │  TIM PWM     RGB LED, sync, fan?   │
       │  GPIO/EXTI   E-stop, motor enable, │
       │              user btn, SD detect    │
       └──┬──────┬──────┬──────┬──────┬─────┘
          │      │      │      │      │
       ┌──▼─┐ ┌──▼──┐ ┌─▼──┐ ┌─▼──┐ ┌▼──┐
       │CAN │ │EBIMU│ │ADS │ │ENC │ │SD │
       │XCVR│ │UART │ │131 │ │AS5 │ │MMC│
       └──┬─┘ │수신기│ │M04 │ │048A│ └───┘
          │   └─────┘ └────┘ └────┘
          │ CANopen (or EtherCAT 옵션)
     ┌────▼─────────┐
     │ Elmo driver  │ → BLDC Motor 1
     │ (모델 미정)  │
     └──────────────┘   (동일 구성 x2)

 Power: 24/36/48V (모터 후 확정)
        ─► 5V buck (4A) ─► 3.3V LDO (디지털)
                          └► 3.3V LDO (아날로그, ferrite 분리)
```

---

## 2. MCU 상세 비교·추천 (사용자 요청 — 장단점 확장)

### 핵심 후보 4종 상세 비교

| 항목 | **STM32H723VGT6** ⭐ | STM32H723ZGT6 | STM32H743VIT6 | STM32H723VGH6 (BGA) |
|---|---|---|---|---|
| **패키지** | LQFP100 | LQFP144 | LQFP100 | UFBGA100 |
| **물리 크기** | 14×14 mm (핀피치 0.5mm) | 20×20 mm (핀피치 0.5mm) | 14×14 mm | **7×7 mm** |
| **PCB 면적** | 196 mm² | 400 mm² | 196 mm² | 49 mm² |
| **납땜 난이도** | ★★ (스텐실 + 핫에어 OK) | ★★ (동일) | ★★ (동일) | ★★★★★ (리플로우 오븐 필수) |
| **리워크** | 가능 (잘못 납땜해도 수정 가능) | 가능 | 가능 | **거의 불가능** (BGA X-ray 필요) |
| **CPU 코어** | Cortex-M7 @ **550 MHz** | M7 @ 550 MHz | M7 @ 480 MHz | M7 @ 550 MHz |
| **CoreMark** | ~3220 | ~3220 | ~2424 | ~3220 |
| **DP-FPU** | ◯ (double precision) | ◯ | ◯ | ◯ |
| **Flash** | 1 MB (single bank) | 1 MB | **2 MB (dual bank)** | 1 MB |
| **RAM 총량** | 564 KB | 564 KB | **1 MB** | 564 KB |
| └ DTCM (zero-wait) | 128 KB | 128 KB | 128 KB | 128 KB |
| └ ITCM | 64 KB | 64 KB | 64 KB | 64 KB |
| └ AXI SRAM | 320 KB | 320 KB | **512 KB** | 320 KB |
| **USB HS PHY 내장** | ◯ ⭐ | ◯ ⭐ | ✗ (외부 ULPI 필수, +25mm²) | ◯ |
| **Ethernet RMII** | ✗ | ◯ | ◯ | ✗ |
| **CAN-FD (FDCAN)** | 2개 | 2개 | 2개 | 2개 |
| **UART** | 8개 | 10개 | 8개 | 8개 |
| **SPI** | 6개 | 6개 | 6개 | 6개 |
| **I²C** | 4개 | 4개 | 4개 | 4개 |
| **타이머 (32-bit)** | 2 | 2 | 2 | 2 |
| **HRTIM (모터 PWM)** | ◯ | ◯ | ◯ | ◯ |
| **ADC (16-bit)** | 3 | 3 | 3 | 3 |
| **DMA 컨트롤러** | GPDMA + BDMA + MDMA | 동일 | 동일 | 동일 |
| **DMA 스트림 수** | 16+ | 16+ | 16+ | 16+ |
| **SDMMC** | 2 | 2 | 2 | 2 |
| **소비전력 (Run typ.)** | ~120 mA @ 550MHz | ~120 mA | ~110 mA @ 480MHz | ~120 mA |
| **VDD** | 1.62~3.6V | 동일 | 동일 | 동일 |
| **온도 등급** | -40~85°C | 동일 | 동일 | 동일 |
| **가격 (개당)** | ~$8~12 | ~$10~14 | ~$12~16 | ~$10~14 |
| **수급 (2026)** | 양호 | 양호 | 양호 | 보통 |
| **ST HAL/CubeMX 지원** | 완전 | 완전 | 완전 (가장 성숙) | 완전 |
| **Nucleo 보드 존재** | ✗ (H723ZG는 있음, 핀맵 일부 호환) | ◯ Nucleo-H723ZG | ◯ Nucleo-H743ZI | ✗ |
| **레퍼런스 회로 풍부도** | ★★★ | ★★★★ | ★★★★★ | ★★ |
| **입문 난이도** | ★★ | ★★ | ★ (HAL 자료 가장 많음) | ★★★★★ |

### 후보별 장단점 요약

#### ⭐ **STM32H723VGT6 (LQFP100)** — 1순위 추천

**장점:**
1. **컴팩트** — 14×14mm, 자가 납땜 가능한 가장 작은 H7 (BGA 제외)
2. **USB HS 내장 PHY** — 외부 ULPI(USB3343 등) 5×5mm 부품 + 라우팅 절약
3. **HRTIM 포함** — 미래에 모터 직결 PWM도 가능 (현재는 CAN-FD 사용)
4. **DMA 충분 + DTCM 128KB** — 1kHz 제어 루프 코드를 zero-wait DTCM에 올려 결정적 실행 가능
5. **CAN-FD 2개** — 1개는 Elmo, 1개는 디버그/추가 센서 여유
6. **Nucleo-H723ZG와 코드 호환** — 라이브러리·예제 그대로 사용
7. **수급 안정·가격 적당**

**단점:**
1. **Ethernet 불가** (RMII 핀 핀맵에 없음) — 우리는 안 쓰니까 무관
2. **Flash 1MB 단일 뱅크** — OTA dual-bank 펌웨어 업데이트는 못 함 (DFU 가능)
3. **RAM 564KB** — 고해상도 비전 처리할 거면 부족 (그건 Jetson이 함)

#### STM32H723ZGT6 (LQFP144) — 2순위

**장점:**
1. Ethernet RMII 핀 있음 (미래에 LAN 추가 가능)
2. UART 10개·핀 여유 — 확장 헤더 풍부
3. Nucleo-H723ZG와 정확히 동일 칩

**단점:**
1. **면적 2배 (400mm²)** — 컴팩트 보드에 직격탄
2. Ethernet 안 쓸 거면 +200mm²·핀 모두 낭비
3. 보드 사이즈가 70×50mm으로 커질 수 있음

#### STM32H743VIT6 (LQFP100) — RAM·Flash 우선 시

**장점:**
1. **RAM 1MB / Flash 2MB** — 큰 펌웨어, 큰 SD 버퍼, OTA dual-bank OK
2. ST HAL 자료·예제 가장 풍부 (가장 인기 H7)
3. Ethernet RMII 가능

**단점:**
1. **USB HS 내장 PHY 없음** — 외부 USB3343 ULPI PHY 칩 + 24MHz 크리스털 + 패시브 = +25~30mm² + BOM 부품 ↑
2. CPU 480MHz로 약간 느림 (보통 무관)
3. 우리 펌웨어는 1MB Flash + 564KB RAM이면 충분 → 오버스펙

#### STM32H723VGH6 (UFBGA100, 7×7) — 극단 컴팩트

**장점:**
1. **49mm², 가장 작음** (H723VGT6 대비 75% 감소)
2. 동일 CPU/메모리/주변기기

**단점 (치명적):**
1. **자가 납땜 사실상 불가** — 리플로우 오븐 + 스텐실 정확도 필수, 핫에어 어려움
2. **리워크 거의 불가능** — 한 번 잘못 납땜하면 X-ray 검사 후 BGA 리볼 또는 칩 버림
3. **PCB 라우팅 난이도 ↑** — 0.5mm 핀피치 BGA → 4-layer로는 fanout 빠듯, 6-layer 필요
4. **디버깅 어려움** — 칩 다리 접근 불가, 테스트 포인트로만 측정
5. **PCB 어셈블리 외주 비용 ↑** (스텐실 + reflow 필수)

→ 학습·시제품 단계엔 **비추**. 양산 단계에서 검토.

### 최종 추천: **STM32H723VGT6 (LQFP100)** 변함 없음

근거 우선순위:
1. **컴팩트 (사용자 필수)** ✓
2. **USB HS 내장 (디버그·DFU 부품 절약)** ✓
3. **자가 납땜·리워크 가능 (학습·시제품 단계)** ✓
4. **성능·메모리 충분** ✓
5. **수급·가격 안정** ✓

> ❓ CubeMX에 다른 H7로 세팅했으면 알려줘. H723VGT/H743VI/H7A3 등은 ST HAL이 거의 동일해서 핀맵 마이그레이션 비용 작음.

### 1순위 추천: **STM32H723VGT6 (LQFP100, 14×14)** ⭐

**이유:**
1. **컴팩트** — LQFP100은 자가 납땜 가능한 가장 작은 STM32H7 패키지 (BGA 제외)
2. **USB HS 내장 PHY** — Jetson/디버그 USB가 외부 부품 없이 동작 (H743V/H7A3는 외부 ULPI PHY 5x5mm 추가 필요 → 컴팩트 손해)
3. **Flash·RAM 충분** — 1MB Flash + 564KB RAM이면 우리 펌웨어(제어, 통신, BLE 브리지, SD logger) + 여유분 OK
4. **CPU 550MHz** — 1kHz 제어 + 32kSPS ADC + DMA 동시 충분
5. **Ethernet 안 갈 거니까 LQFP144 불필요** → 핀·면적 낭비 X
6. **가격 안정·수급 양호** — 2025년 기준 ST 공식 라인업
7. **Nucleo-H723ZG 보드와 동일 패밀리** — schematic·라이브러리 그대로 활용 (핀맵만 줄이면 됨)

**비추천 이유:**
- BGA(VGH6): 컴팩트엔 좋지만 자가 디버깅·리워크 거의 불가능, 학습 단계엔 비추
- LQFP144(ZGT6): Ethernet 안 쓸 거면 +200mm² 헛 비용
- H743V: 외부 USB PHY 부품 → 컴팩트 모순
- H730: 외부 Flash 필요 → 컴팩트 모순

> ❓ CubeMX에서 이미 다른 칩으로 세팅했으면 알려줘. 같은 H7 계열이면 핀맵 호환성 높음.

---

## 3. 부품 리스트 v2

### 3-1. 전원 — 보드 통과 모터 전원 공급 (사용자 확정)

> **변경:** 모터 전원이 **보드를 통과**한다 → DC 입력은 모터 전체 전류를 견뎌야 함.
> XT30 (15A 연속, 30A 피크) 또는 더 큰 커넥터 필요. JST-GH는 부적합.

#### 전원 토폴로지

```
24/36/48V DC  ──► XT30 입력 ──► 보호회로 ──► 모터 전원 분기 ───► Elmo Driver 1
                                  │                       └──► Elmo Driver 2
                                  │
                                  ├──► 24V→5V Buck ──► 5V→3.3V LDO ──► MCU/센서
                                  │
                                  └──► (옵션) 24V→12V if needed
```

#### DC 입력 (모터 전원 통과)

- **커넥터: XT30PW (PCB 마운트, 15A 연속, 30A 피크)** — 15×9 mm
  - 대안 1: XT60 (30A 연속) — 더 크지만 60A까지 안전
  - 대안 2: Molex Mini-Fit Jr. 4-pin (13A/pin) — 산업용
  - **선택 기준: 모터 최대 연속 전류 × 1.5 마진**. Elmo 모터 합산 12A면 XT30, 20A 넘으면 XT60.
- **+V 분기**: 모터로 가는 라인은 보드 outlet 쪽으로 별도 출력 단자
  - 출력 커넥터: XT30PW × 2개 (모터당 1개) 또는 Phoenix MSTBA 등
  - **트레이스 폭: 100mil (2.5mm) 외층 1oz/ft² 기준 ~10A** — 더 굵게 또는 폴리곤 채움

#### 보호회로 (모터 전류 통과 → 강화)

| 부품 | 사양 | 이유 |
|---|---|---|
| 역전압 | **N-MOSFET ideal diode (e.g. AOD514 + LTC4359)** | 15A 통과 가능, 저저항 (P-MOS는 큰 전류 어려움) |
| 입력 TVS | SMCJ58A (1500W) | 인덕티브 킥백 흡수 |
| 퓨즈 | **자동차용 ATM mini-blade 15A** 교체형 | 폴리퓨즈는 15A에 너무 큼 |
| 역전압 LED 인디케이터 | 적색 0805 | 시각적 확인 |
| 입력 벌크 캡 | **470µF/63V 알루미늄 polymer × 2** + 22µF 세라믹 | 모터 회생 전류 흡수, 전압 변동 평활 |
| Common-mode choke | Wurth 744232 또는 동급 (전류 ≥15A) | EMI |
| eFuse (옵션 권장) | **TPS25940/LTC4368** | 과전압·과전류·역전압 통합, 비싸지만 안전 |

#### 로직 전원 (24V→5V→3.3V) — 모터 라인에서 분기

- **DCDC 24/48V→5V Buck: TPS62933 (2.5MHz, 60V, 2A)** — 모터 전원에서 분기
  - 입력 캡: 모터 전원 단과 분리 (별도 ferrite로 노이즈 격리)
- **LDO 5V→3.3V 디지털: TLV75533 (SOT-23-5, 500mA)**
- **LDO 5V→3.3V 아날로그: 별도 ferrite + 동일 LDO** — ADS131M04 정밀도용
- **각 LDO 출력 1µF 세라믹 + MCU 디커플링 100nF × 다수**

#### 모터 전원 영역 격리 (PCB 레이아웃)

- 모터 전원 라인 = **굵은 폴리곤** (보드 한쪽에 몰아두기)
- 로직 GND ↔ 파워 GND = **단일점 연결** (star ground)
- 모터 전원 ↔ ADC/통신 영역 사이 = **2~3mm 간격** + ferrite/필터
- 모터 트레이스 위 다른 신호 라우팅 금지 (인덕티브 결합 차단)

#### 사이즈 영향

- XT30 입력 + 출력 ×2 = 약 **180 mm²** (vs JST-GH 50mm²)
- 보호회로 강화 (큰 캡, eFuse, choke) = 약 **+150 mm²**
- **총 추가: ~280 mm² → 보드 사이즈 50×50 → 55×50mm 또는 50×55mm 권장**

### 3-2. MCU
- **STM32H723ZGT6**
- HSE 25 MHz crystal (USB HS, Ethernet 정확도 위해 25MHz)
- LSE 32.768 kHz (RTC 옵션)
- VCAP 캡 (H7 내부 LDO 안정화)
- VBAT (코인셀 옵션)
- BOOT0 점퍼
- NRST 버튼 + 100nF
- **SWD 10-pin Cortex Debug** (TRACESWO 포함 — H7 디버깅 강력)

### 3-3. 모터 인터페이스 (Elmo 가정, 모델 미확정)
- **CAN-FD 트랜시버 x1**: TCAN1462 (5Mbps FD)
- **CAN 종단 120Ω + 점퍼**
- **CAN ESD**: PESD2CAN
- **CAN 커넥터**: JST-XH 5pin (CAN_H/L/GND/SHIELD/+V_logic)
- **Estop 입력**: 광커플러 (PC817) — 외부 안전 회로 격리
- **Motor power enable 출력**: 로직-레벨, MCU 제어 (모터 전원 라인 차단용 외부 컨택터/SSR 트리거)
- **EtherCAT 사용 안 함** (사용자 확정). 모터-MCU 간 통신은 CAN-FD/CANopen 단일 경로
- **CAN 커넥터: JST-GH 5pin (1.25mm)** — XH 대신 GH로 사이즈 절감

### 3-4. 센서

#### 로드셀 (강화)
- **ADS131M04** (24-bit ΔΣ, 32kSPS, 4ch 동시 샘플, SPI) ← HX711 대체
  - 두 케이블 force + 예비 2채널
  - 외부 2.5V 정밀 레퍼런스 (REF5025) 또는 내장
  - 차동 입력 + RC 안티앨리어싱 필터
- 로드셀 커넥터: JST-XH 4pin x2 (E+, E-, S+, S-)

#### 케이블 위치 인코더
- **AS5048A** (14-bit 자기식, SPI) x2 — 풀리에 다이아메트럴 자석
- 또는 모터 드라이버 내부 인코더 피드백을 CANopen으로 받기 (Elmo 옵션)

#### IMU
- **온보드 IMU 없음** — EBIMU (E2BOX 무선) 사용
- **EBIMU 수신기 UART 입력**: 4-pin JST-GH (TX/RX/GND/3V3)
  - 수신기를 보드 USB 5V로 전원 공급도 가능 (5V 라인 별도 핀)

#### 기타 모니터링
- **모터 전류 센스**: INA240A1 x2 (외부 션트 0.5mΩ) — 드라이버 외부에서 추가 측정 시
- **NTC 10k** x2 — 모터/풀리 온도
- MCU 내부 온도/Vrefint

### 3-5. Jetson 통신 (1순위 USB HS, 옵션 Ethernet/UART)
- **USB Type-C 커넥터** (CC 5.1k 풀다운만, PD 없음)
- ESD: TPD4S012 또는 USBLC6
- D+/D- → STM32 OTG_HS 직결
- VBUS → 5V (자체 전원 우선이면 별도 분리)
- (옵션) UART backup: JST-GH 4pin

### 3-6. 저장
- **microSD push-push 슬롯**, **SDMMC 4-bit 모드**
- 카드 detect 핀 → MCU GPIO
- 풀업 저항, 100nF
- TVS (선택)

### 3-7. UI / E-stop (필수 — 사용자 확정)

#### E-stop 회로 (이중 안전)

```
┌──────────────┐         ┌─────────────┐
│ External NC  │  J_ESTOP│ Pull-up     │
│ Mushroom Btn │─────────│ + 디바운스   │──┬──► MCU EXTI (SW 차단)
└──────────────┘         │ R = 10k,C=10nF│  │
                         └─────────────┘  │
                                          ├──► AND gate 입력 (HW 차단)
                                          │
            MCU motor_enable GPIO ────────┴──► AND gate 입력
                                          │
                              motor_enable_final ◄ AND gate 출력
                                          │
                                          ▼
                              모터 드라이버 enable / SSR 게이트
```

**왜 이중인가:**
- **SW 차단** = MCU EXTI 인터럽트로 즉시 disable 명령 송신, 빠른 응답
- **HW 차단** = MCU가 멈춰도(펌웨어 행, SEGGER 정지) AND 게이트가 직접 enable 신호 끊음 → MCU 우회 안전

**부품:**
- 외부: 표준 산업용 mushroom NC 버튼 (사용자 마련)
- 보드: JST-GH 2pin (NC 접점 입력), 10kΩ 풀업, 10nF 디바운스 캡, 1k 직렬 저항
- AND gate: **74LVC1G08 (SOT-353, 1.6×1.6mm)** — 1게이트, 매우 작음
- E-stop 상태 LED (적색 0603)
- (옵션) RS-latch — 한 번 누르면 reset 버튼 누를 때까지 락. SR 래치 (74LVC1G373) 1개 추가
- 면적 추가: ~25 mm²

#### 그 외 UI
- RGB LED (TIM PWM 3채널) — 시스템 상태
- Sync LED — SYNC 펄스 시각화
- USER 버튼 + RESET 버튼
- E-stop reset 버튼 (래치 사용 시)
- 부저 (옵션, 컴팩트 위해 보류)

### 3-8. 무선 제어 — 안테나 PCB 내장 vs IPEX 외부 (사용자 질문 답)

| 옵션 | 모듈 | 장점 | 단점 |
|---|---|---|---|
| **PCB 내장** | ESP32-C3-MINI-1 | • 단일 모듈, 외부 부품 0<br>• 비용 저렴<br>• 조립 단순<br>• 깔끔한 외형 | • **15mm × 15mm keep-out** 영역 필수 (구리·부품 X)<br>• 안테나가 보드 가장자리에 와야 함<br>• 케이스에 가리면 신호 약화 (특히 금속 케이스)<br>• 사용자 몸이 안테나 가리면 BLE 끊김 가능 |
| **IPEX 외부** ⭐ | ESP32-C3-MINI-1**U** + IPEX 케이블 + 외부 안테나 (예: 2.4GHz dipole) | • **케이스 외부에 안테나 부착 가능** (착용 중 신호 안정)<br>• 보드 keep-out 영역 작음<br>• BLE 연결 거리 길고 안정적<br>• 안테나 방향 자유 | • IPEX 케이블 + 외부 안테나 부품 필요<br>• 비용 ↑<br>• 안테나 케이블이 보드 외부로 나가야 함 |

**Wearable Exosuit 추천: IPEX 외부 안테나 (`-1U`)**

이유: 사용자가 옷 입으면 보드는 케이스 안 + 옷 속 → PCB 내장 안테나 신호 거의 다 막힘. 외부 안테나만 살짝 옷 밖에 노출하면 BLE/WiFi 안정. "실시간 무선 통제"가 목적이면 IPEX가 정답.

**부품 (IPEX 선택 시):**
- ESP32-C3-MINI-1**U** — UART (1Mbps) + GPIO 2개 (EN, IO9)
- IPEX MHF1 케이블 (10~30cm)
- 2.4GHz 안테나 (예: 무지향성 펜형, 5dBi)
- ESP-IDF 펌웨어 별도 작성

(대안) UART 헤더만 두고 외부 BLE 모듈 플러그 — 더 작지만 결합도 ↓

### 3-9. 디버그
- SWD 10-pin Cortex
- VCP UART (USB가 안 될 때 fallback)
- 테스트 포인트: 3V3, 5V, GND, CAN_H, CAN_L, MCU 주요 신호

---

## 4-8. Jetson ↔ MCU 데이터 플로우 (신규)

> Jetson에서 카메라 → 포즈 추정 → 관절 각/속도/타겟 토크를 MCU로 다운링크.
> MCU는 그 값을 reference로 받아 force/admittance 제어 루프 실행.
> 거꾸로 MCU → Jetson 업링크는 모터 상태/로드셀/IMU 텔레메트리.

### 데이터량 추정

| 방향 | 항목 | 패킷 크기 | 주기 | 대역 |
|---|---|---|---|---|
| **Jetson→MCU** | pose 20관절 × (3각+3속) × 4B + 타겟·모드 | ~1 KB | 60~100 Hz | 60~100 KB/s |
| **MCU→Jetson** | 모터 2 × state(20B), force 4ch×4B, IMU echo | ~200 B | 1 kHz | 200 KB/s |
| **합계** | | | | **< 500 KB/s** |

→ **UART 1Mbps 도 이론적으로 충분.** 속도가 아니라 **지연(latency)·jitter·동기**가 핵심.

### 지연 요구
- 카메라 캡처 → 포즈 → MCU 도착 → 제어 반영 < **20 ms 목표** (착용감)
- MCU 측 처리 < 1 ms (1kHz 루프)
- 통신 채널이 < 5 ms jitter 보장해야 함

### 하드웨어 동기 라인 — 왜 필요한가 (사용자 질문 답)

**문제:** Jetson이 카메라 프레임에서 포즈를 뽑아 MCU에 보낼 때, 그 데이터가 *언제 시점의* 정보인지 MCU가 정확히 모름.

```
T1: 카메라 프레임 캡처 (Jetson clock)
T2: pose 추정 끝 (T1 + ~10ms, GPU jitter)
T3: UART 전송 시작 (T2 + ~2ms, 스케줄러 jitter)
T4: MCU 도착 (T3 + 통신 시간)
T5: MCU 제어 루프 사용
```

MCU는 T5 시점에 받은 데이터를 보고 모터를 명령하지만, 그 데이터는 사실 **T1 시점의 사용자 자세**. T5−T1 = "데이터 나이". 이걸 모르면:
- 사용자가 빠르게 움직일 때 **몇 ms 늦은 명령**을 그대로 따라가며 진동·불안정
- Force feedback과 pose feedback이 시간축에서 안 맞아 admittance 제어 망가짐

**SYNC 라인의 역할:** MCU와 Jetson이 **공유하는 하드웨어 이벤트**를 만들어서 둘의 시계를 맞춤.
- 방법 A — MCU가 1kHz/100Hz 펄스 출력 → Jetson이 GPIO로 받아 자기 시계와 매핑 (가장 흔함)
- 방법 B — Jetson이 카메라 트리거 펄스 → MCU EXTI로 받아 "이 시점에 카메라 셔터 열림" 기록
- 둘 다 가능하게 양방향 GPIO 헤더 권장

**비유:** 두 사람이 다른 손목시계 쓰면서 "3시에 만나자" 하면 시간이 안 맞음. 동시에 손뼉치면(SYNC) 그 순간을 기준으로 시계 보정 가능.

**부품 비용:** GPIO 핀 2개 + JST-GH 3pin 커넥터 1개 ≈ **30 mm²**. 거의 공짜.
**없으면:** 소프트웨어 NTP 같은 시간 보정 — 복잡하고 jitter 큼. HW SYNC가 정답.

### 결론
- SYNC_OUT (MCU→Jetson) + SYNC_IN (Jetson→MCU) 두 핀 모두 헤더에 노출
- 처음엔 SYNC_OUT만 써도 충분. SYNC_IN은 카메라 트리거 동기 시 사용
- 핀: 3-pin JST-GH (SYNC_OUT / SYNC_IN / GND)

### 통신 채널별 적합성 재검토 (포즈 다운링크 고려)

| 방식 | 지연 | jitter | HW 동기 결합 | 평가 |
|---|---|---|---|---|
| 고속 UART (DMA + idle line) | < 1ms | 매우 낮음 | 쉬움 (별도 GPIO) | ⭐ 단순·결정적 |
| USB HS CDC | 1~5ms | 중간(호스트 의존) | 별도 GPIO 필요 | △ |
| Ethernet UDP | < 1ms | 낮음 | PTP 가능 (복잡) | ⭐ 산업표준 |
| Ethernet TCP | 가변 | 높음 | — | ✗ (재전송 지연) |

**재추천: 고속 UART(DMA) + 별도 SYNC GPIO** 1순위.
- 부품 거의 안 늘어남, 결정적, DMA로 무손실, Jetson UART 핀에 직결.
- Ethernet은 미래 확장(여러 Jetson, 원격 모니터링) 위해 PHY 풋프린트만 DNF로 남길지 결정.

### Link health watchdog
- MCU는 마지막 Jetson 패킷 수신 시각 추적 → **N ms 무수신 시 motor enable 차단** (안전 fail-safe)
- Jetson 측도 MCU 텔레메트리 끊김 감지

---

## 4-9. DMA 활용 매핑 (필수)

> STM32H7는 GPDMA, BDMA, MDMA 다중 DMA 컨트롤러. 모든 고속 주변기기는 DMA로 돌려 CPU는 제어 알고리즘에만 집중.

| 주변기기 | DMA | 방향 | 트리거 | 이유 |
|---|---|---|---|---|
| **SPI1 ↔ ADS131M04** | DMA1 stream | RX/TX 양방향 | SPI RX 인터럽트 | 24bit×4ch×32kSPS = 384KB/s, CPU 부담 큼 |
| **SPI3 ↔ AS5048A x2** | DMA1/2 | RX/TX | 타이머 트리거 (1kHz) | 1kHz 위치 샘플링 |
| **SDMMC1** | 내장 IDMA | R/W | 자체 | SD 카드 4-bit 50MHz 쓰기 |
| **USART (Jetson, USB CDC fallback)** | DMA | TX/RX | idle line + DMA | 무손실 스트리밍, 패킷 경계 idle 감지 |
| **USART (EBIMU)** | DMA | RX | idle line | 무선 IMU 패킷 무손실 |
| **USART (ESP32 BLE/WiFi)** | DMA | TX/RX | idle line | 무선 명령/텔레메트리 |
| **ADC1/2 (전류·온도)** | DMA | RX | 타이머 트리거 | 1kHz 동기 샘플 |
| **TIM PWM (RGB LED)** | DMA | (옵션) | 업데이트 | 색상 시퀀스 |
| **FDCAN1** | (자체 메시지 RAM) | — | 인터럽트 | DMA 불필요 |
| **USB OTG_HS** | (자체 DMA) | R/W | endpoint | 내장 |

**검증 사항:** STM32H7 DMA 스트림은 한 채널에 한 주변기기만 매핑 가능 → CubeMX에서 충돌 검사 필수.
픽셀맵·핀맵 확정 시 DMA request mux도 같이 freeze.

---

## 4-10. 사이즈 / 폼팩터 예산 (모터 전원 통과 반영, v2.5)

| 블록 | 면적 (대략) | 비고 |
|---|---:|---|
| MCU H723VGT6 LQFP100 (14×14) | 196 mm² | 확정 |
| ESP32-C3-MINI-1**U** | 219 mm² | + 안테나 keep-out (IPEX는 작게 OK) |
| microSD push-push | 210 mm² | 필수 |
| ADS131M04 TQFP-32 + REF | 80 mm² | |
| **DC 입력 XT30 + 출력 XT30 ×2** | **180 mm²** | 모터 전원 통과 |
| **모터 전원 보호회로 강화** | **~150 mm²** | eFuse, 큰 캡 ×2, choke |
| Buck (TPS62933 + 인덕터·캡) | 100 mm² | 2.5MHz |
| LDO x2 + 캡 | 50 mm² | |
| USB-C 커넥터 | 80 mm² | DFU·디버그 |
| CAN 트랜시버 + ESD | 30 mm² | |
| 엔코더 IC AS5048A x2 | 60 mm² | |
| 인터커넥트 (JST-GH x6: CAN, Jetson UART, SYNC, EBIMU, Loadcell ×2, Estop) | ~180 mm² | |
| **E-stop 회로 (AND gate, latch, debounce)** | **~25 mm²** | |
| 디커플링 캡, 패시브 | ~300 mm² | 양면 분산 |
| **부품 합계** | **~1860 mm²** | (+ 모터 전원 영향 ~280) |
| × 1.5 (라우팅·간격·모터 트레이스 굵기) | **~2800 mm²** | |

**수정 목표 보드:**
- **1차 (안전·추천):** **55 × 55 mm = 3025 mm²** · 4-layer · 모터 전원 영역 격리
- **공격적:** 50 × 50 mm = 2500 mm² · 6-layer · 양면 실장
- **참고:** Moteus r4.5 = 46×53mm (단, 모터 드라이버 내장이라 우리와 다름)

**컴팩트 절감 옵션 (이미 적용된 것):**
| 액션 | 적용 여부 |
|---|---|
| Ethernet PHY+RJ45 제거 | ✅ 적용 (-340 mm²) |
| MCU LQFP144→LQFP100 | ✅ 적용 (-204 mm²) |
| Buck 고주파 (TPS62933) | ✅ 적용 (-50 mm²) |
| 모든 커넥터 GH (CAN/센서/EBIMU/SYNC) | ✅ 적용 |

**모터 전원 통과로 인한 추가 (불가피):**
| 항목 | 추가 |
|---|---:|
| XT30 in/out ×3 (입력+모터 ×2) | +180 mm² |
| eFuse + 큰 캡 + choke | +150 mm² |
| 굵은 트레이스 폴리곤 | 표면적은 X, 라우팅 마진 ↑ |

---

## 4. Phase 플랜 (v2)

| Phase | 내용 | 산출물 | 상태 |
|---|---|---|---|
| **P0** | 요구·블록도 (이 문서 v2) | `PLAN.md` | ✅ |
| **P1** | **각 블록 reference schematic 수집** ← 형이 말한 거 | `refs/reference_schematics.md`, 데이터시트 PDF | 🟡 |
| **P2** | BOM v1 부품 LCSC/Mouser 검증 (재고/가격) | `parts/BOM_v1.csv` | ⬜ |
| **P3** | KiCad 프로젝트 + 라이브러리 (심볼/풋프린트) | `kicad/exo_stm32.kicad_pro` | ⬜ |
| **P4** | 블록별 schematic (모터 IF는 일반화 버전) | `*.kicad_sch` | ⬜ |
| **P5** | **모터/Elmo 드라이버 확정 → 모터 IF 블록 + 전압 확정** | 결정 로그 | 보류 |
| **P6** | ERC + schematic 검토 → freeze | tag `sch-v1` | ⬜ |
| **P7** | PCB 레이아웃 (4-layer: SIG/GND/PWR/SIG) | `*.kicad_pcb` | ⬜ |
| **P8** | DRC, gerber, 발주 | gerber zip | ⬜ |
| **P9** | 브링업 | 체크리스트 | ⬜ |

---

## 5. 사용자 답변 반영 결정 (v2.4)

이번 라운드로 다음 항목 확정 또는 권장 결정:

| 항목 | 결정 | 이유 |
|---|---|---|
| MCU | **STM32H723VGT6 (LQFP100, 14×14)** | §2 비교표 — 컴팩트 + USB HS 내장 + 성능 충분 |
| Ethernet | **완전 제거** (사용자: "안할 듯") | 340mm² 절약 |
| Jetson 통신 | **고속 UART + HW SYNC GPIO** (사용자 동의) | §4-8 SYNC 설명 |
| USB-C | **포함** (작게, 보드 모서리 배치) | DFU·디버그 매우 편함, 80mm² 가치 있음 |
| ESP32 안테나 | **IPEX 외부 (`-1U`)** | wearable에선 외부 안테나가 BLE 안정 (§3-8) |
| E-stop | **포함, 이중 안전 (SW EXTI + HW AND gate)** | 사용자 "긴급정지도 만들어야 해" |

## 6. 아직 남은 결정 (v2.5)

| 항목 | 옵션 | 권장 |
|---|---|---|
| 사이즈 목표 | (a) 55×55mm 4L (안전) / (b) 50×50mm 6L (공격적) | (a) 1차 시제품 |
| CubeMX 칩 | H723VGT6로 진행 OK? | 사용자 확인 필요 |
| 모터·전압·전류 (Phase 5) | Elmo 모델 + 24/36/48V + max 전류 | 모터 확정 후 |
| eFuse 사용 | TPS25940 등 통합 보호 IC vs 개별 부품 | eFuse 추천 (안전 + 컴팩트) |

---

## 6. 모터 미확정 상태에서 진행 가능한 것

- 전원 입력 단(보호회로): 60V 입력 가능한 부품으로 24/36/48V 모두 커버
- CAN-FD 트랜시버: Elmo 모델 무관 동일
- 모터 전원 enable: 외부 SSR/컨택터 트리거 신호만 핀 할당
- Estop, 워치독, 보호 회로
- MCU/USB/SD/IMU 입력/로드셀/엔코더: 모터와 무관 → **전부 확정 가능**

→ Phase 5 외 모든 단계가 선행 가능.
