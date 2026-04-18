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

## 2. MCU 후보 (컴팩트 고려)

| 후보 | 패키지 | 면적 | Ethernet 가능 | USB HS 내장 PHY | 평가 |
|---|---|---:|---|---|---|
| STM32H723**Z**GT6 | LQFP144 (20×20) | 400 mm² | ◯ | ◯ | Ethernet 갈 때만 |
| **STM32H723VGT6** | **LQFP100 (14×14)** | **196 mm²** | ✗ | ◯ | ⭐ Ethernet 안 가면 1순위 |
| STM32H723VGH6 | UFBGA100 (7×7) | 49 mm² | ✗ | ◯ | 가장 작음, BGA — 자가 납땜·리워크 어려움 |
| STM32H743VIT6 | LQFP100 (14×14) | 196 mm² | ✗ | ✗ (외부 ULPI 필요) | USB HS 위해 PHY 추가 → 컴팩트 X |

**현재 추천: STM32H723VGT6 (LQFP100)** — Ethernet 미사용 가정, USB HS 내장 PHY, 반-크기.

> ❓ CubeMX 세팅 어떤 칩이야? 그 칩으로 맞춤. (현재 V100 가정 vs 이미 Z144로 했으면 알려줘)

---

## 3. 부품 리스트 v2

### 3-1. 전원 (컴팩트 버전)
- DC 입력: **JST-GH 2pin** (전압은 모터 후 결정) — XT30 대신 SMD GH로 사이즈↓
- 역전압: P-MOSFET ideal diode (SI4435 SO-8)
- 보호: SMBJ TVS (SMB), Polyfuse 5A (1812)
- DCDC 24V→5V: **TPS62933 (2.5MHz, 60V 입력, 2A)** — 인덕터·캡 소형화 (TPS54360 대비 면적 ~30% ↓)
  - (대안: 모듈 TPSM84A21 — 인덕터 통합, 더 작고 비쌈)
- LDO 5V→3.3V 디지털: **TLV75533** (SOT-23-5, 500mA)
- LDO 5V→3.3V 아날로그: 별도 ferrite + 동일 LDO (ADC 정밀도)
- 벌크: 47µF/63V SMD x1 + 22µF/16V + 디커플링 (전부 SMD)
- 전원 LED만 유지, 전류 모니터는 옵션 제거 (컴팩트 우선)

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

### 3-7. UI / 안전
- RGB LED (TIM PWM 3채널)
- Sync LED
- USER 버튼 + RESET 버튼
- **하드웨어 E-stop 단자대** (NC 접점 입력 → MCU + 모터 enable 라인 직접 차단)
- 부저 (옵션)

### 3-8. 무선 제어 (신규)
- **ESP32-C3-MINI-1 (또는 -1U 외부 안테나)** — BLE 5.0 + WiFi 802.11b/g/n, 자체 인증 안테나 내장
- MCU ↔ ESP32 인터페이스: **UART (1Mbps)** + GPIO 2개 (RESET, BOOT0/IO9)
- ESP32는 별도 펌웨어 (ESP-IDF) — STM32에서 BLE/WiFi 가상화
- 안테나 keep-out 영역 PCB에 명시 (15mm clearance)
- 전원: 3.3V 500mA (TX 피크), 별도 LDO 또는 main 3.3V (대용량 캡 필수)
- (대안) UART 헤더만 두고 외부 BLE 모듈(Adafruit Bluefruit, HM-19) 플러그 가능하게

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

### 하드웨어 동기 라인 (필수 추가)
- **SYNC_OUT (MCU GPIO → Jetson GPIO)**: MCU가 일정 주기 펄스 발생 → Jetson이 카메라 프레임 타임스탬프와 매칭
- **SYNC_IN (Jetson GPIO → MCU EXTI)**: 카메라 트리거 신호 (옵션, 카메라가 GPIO 트리거 지원 시)
- 핀: 2-pin JST + GND. 기존 sync LED 핀 재활용 가능.

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

## 4-10. 사이즈 / 폼팩터 예산 (컴팩트 필수)

| 블록 | 면적 (대략) | 비고 |
|---|---:|---|
| MCU LQFP100 (14×14) | 196 mm² | LQFP144 선택 시 +204mm² |
| ESP32-C3-MINI-1 | 219 mm² | + 안테나 keep-out 15mm |
| microSD push-push | 210 mm² | 필수 |
| ADS131M04 TQFP-32 + REF | 80 mm² | |
| Buck (TPS62933 + 인덕터·캡) | 100 mm² | 2.5MHz로 축소 |
| LDO x2 + 캡 | 50 mm² | |
| USB-C 커넥터 | 80 mm² | DFU·디버그 |
| CAN 트랜시버 + ESD | 30 mm² | |
| 엔코더 IC AS5048A x2 | 60 mm² | |
| 인터커넥트 (JST-GH x6) | ~180 mm² | 모두 1.25mm GH로 통일 |
| 디커플링 캡, 패시브 | ~300 mm² | 양면 분산 |
| **부품 합계** | **~1500 mm²** | |
| × 1.5 (라우팅·간격) | **~2300 mm²** | |

**목표 보드:**
- **1차 (안전):** 50 × 50 mm = 2500 mm² · 4-layer · 모든 부품 단면
- **공격적:** 45 × 40 mm = 1800 mm² · 6-layer · 양면 실장
- **참고:** Moteus r4.5 = 46×53mm (오픈소스 STM32G4 + CAN-FD 보드)

**컴팩트 절감 옵션:**
| 액션 | 절감 |
|---|---:|
| Ethernet PHY+RJ45 제거 | ~340 mm² |
| MCU LQFP144→LQFP100 | ~204 mm² |
| USB-C 제거 (디버그는 SWD만) | ~80 mm² |
| Buck 모듈(TPSM84A21) 사용 | ~50 mm² |
| BGA MCU 사용 | ~150 mm² (단 리워크 어려움) |

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

## 5. 즉시 결정 필요 (컴팩트 우선 반영)

1. **사이즈 목표**: (a) 50×50mm 4L 안전 / (b) 45×40mm 6L 공격적 — 어느 쪽?
2. **MCU 패키지**:
   - **(a) LQFP100 H723VGT6 ← 컴팩트 추천** (Ethernet 포기)
   - (b) LQFP144 H723ZGT6 (Ethernet 살림)
   - (c) BGA UFBGA100 (가장 작음, 자가 납땜·리워크 어려움)
3. **Ethernet 완전 제거 OK?** (예 = 사이즈 절감 max, 아니오 = LQFP144 유지 + RJ45 영역 확보)
4. **Jetson 통신**: (a) UART+SYNC ← 추천 / (b) USB HS / (c) Ethernet
5. **USB-C 디버그·DFU** 포함? (포함 권장, 80mm² 비용)
6. **ESP32 안테나**: PCB 내장(`-1`) vs IPEX 외부(`-1U`)?
7. **E-stop** 외부 안전 회로 유무

---

## 6. 모터 미확정 상태에서 진행 가능한 것

- 전원 입력 단(보호회로): 60V 입력 가능한 부품으로 24/36/48V 모두 커버
- CAN-FD 트랜시버: Elmo 모델 무관 동일
- 모터 전원 enable: 외부 SSR/컨택터 트리거 신호만 핀 할당
- Estop, 워치독, 보호 회로
- MCU/USB/SD/IMU 입력/로드셀/엔코더: 모터와 무관 → **전부 확정 가능**

→ Phase 5 외 모든 단계가 선행 가능.
