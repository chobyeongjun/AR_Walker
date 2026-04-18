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

#### STM32H723VGH6 (UFBGA100, 7×7) — 외주 조립 시 재평가 (사용자 질문)

**전제:** 사용자는 PCBA 외주 (JLCPCB, PCBWay 등) → "자가 납땜 불가"는 deal-breaker 아님.

**성능 — LQFP100과 100% 동일**
- 같은 다이(silicon), 같은 CPU/RAM/Flash/주변기기
- CoreMark, MFLOPS, DMA, USB HS — 전부 동일
- **성능 손실 0**
- 오히려 BGA가 신호무결성(SI)에 미세하게 유리:
  - 핀 길이 짧음 → 인덕턴스/캡 작음
  - 고속 신호(USB HS, 빠른 SPI)에서 LQFP보다 덜 ringing

**그럼에도 BGA가 비추되는 진짜 이유 (외주 조립 가정):**

| 항목 | LQFP100 | BGA UFBGA100 | 영향 |
|---|---|---|---|
| 보드 면적 | 196 mm² | 49 mm² | BGA가 147 mm² 절약 |
| PCB fab 비용 | 표준 4L | **HDI (microvia) 6L 필수** | +$30~60/board |
| 0.5mm vs 0.4mm 피치 | 0.5mm OK | 0.4mm 핀피치 → via-in-pad + epoxy fill | +$15~30/board |
| PCBA assembly 비용 | 표준 SMT | BGA + X-ray 검수 필수 | +$5~15/board |
| 디버깅 (오실로 측정) | 핀 직접 프로빙 가능 | **불가** (테스트 포인트 미리 깔아야) | 시제품 단계 큰 단점 |
| 리워크 | 가능 | **사실상 불가** (전용 BGA 리워크 스테이션 필요) | 잘못 만들면 보드 폐기 |
| 첫 시제품 turnaround | 빠름 | HDI 때문에 fab 리드타임 길어짐 | 1~2주 추가 가능 |
| 단일 볼 불량 | 없음 | 한 볼 안 붙으면 진단 어려움 (X-ray) | 양품률 ↓ |

**재계산: BGA 시 추가 비용 ~$50~100/board** (시제품 5장 만들면 $250~500 추가)

**보드 사이즈 관점:**
- 우리 보드 사이즈 결정 요인은 **microSD(210mm²), ESP32(219mm²), 커넥터(~180mm²), XT30 입출력(180mm²)** 가 다 큼.
- MCU가 196 → 49로 줄어도 보드 전체 사이즈는 거의 안 줄어듬.
- **MCU는 사이즈의 bottleneck이 아님** — BGA로 가도 보드는 50×50→48×50 정도 줄어들 뿐.

**그럼 언제 BGA가 정답?**
1. 진짜 극단 컴팩트 (스마트워치급 25×25mm) — 우리 시나리오 아님
2. 양산 (1000장 이상) — 단가 차이 묻힘
3. 고속 USB 3.0 / DDR3 등 SI가 핵심 — 우리 USB HS는 LQFP로 충분

**결론 (외주 조립 가정 후 재평가):**
- LQFP100 1순위 추천 **유지**
- BGA의 성능 이점 = 0
- BGA의 사이즈 이점 = 우리 보드에서 효과 작음 (MCU가 bottleneck 아님)
- BGA의 비용·리스크 = 명확히 큼

→ 학습·시제품 단계엔 **여전히 LQFP100 추천**. 추후 양산 검토 시 BGA 전환 가능 (펌웨어 100% 호환).

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

### 3-1. 전원 — 모터 전원 보드 통과 + **모터당 15A 연속** (사용자 확정)

> **사용자 정보:** 모터당 **15A 이상 연속** 왔다갔다 → 보드 입력 총 **30A 연속, 50~60A 피크** (가속 시).
> XT30(15A 연속)은 부족. XT60 또는 더 큰 커넥터 필요. 모든 보호회로 30A+ 대응.

#### 전류 예산

| 라인 | 연속 | 피크 (가속/회생) | 비고 |
|---|---:|---:|---|
| 모터 1 | 15 A | 30 A | 사용자 확정 |
| 모터 2 | 15 A | 30 A | 동일 |
| **DC 입력 합** | **30 A** | **50~60 A** | 두 모터 동시 가속 시 |
| 로직 (MCU+센서+ESP32+SD) | < 0.5 A | 1 A | Buck에서 분기 |

#### 전원 토폴로지

```
24/36/48V DC ──► XT60 입력 ──► [eFuse + ideal diode + bulk] ──┬──► XT30 (모터1) ──► Elmo 1
   (30A 연속)                                                  │
                                                              ├──► XT30 (모터2) ──► Elmo 2
                                                              │
                                                              └──► [ferrite] ──► 5V Buck ──► 3.3V LDO
                                                                                                ▼
                                                                              MCU / ADC / ESP32 / SD
```

#### DC 입력 커넥터 — **XT60 권장 (XT30 부족)**

| 옵션 | 연속 | 피크 | 면적 | 평가 |
|---|---:|---:|---:|---|
| XT30PW | 15 A | 30 A | 60 mm² | **부족** (입력 30A 못 견딤) |
| **XT60PW ⭐** | **30 A** | **60 A** | 95 mm² | **추천** — 마진 적당 |
| XT90 | 45 A | 90 A | 160 mm² | 과잉, 사이즈 큼 |
| AS150 | 70 A | 150 A | 250 mm² | 매우 과잉 |
| Molex Mini-Fit Jr 4P | 4×8.5A=34A | — | 200 mm² | 산업용, 큼 |

→ **DC 입력: XT60PW** (30A 연속, 마진 0%지만 일시 OK / 권장은 XT90이면 더 안전)
→ **모터 출력: XT30PW × 2** (모터당 15A 연속이면 마진 0% — XT60으로 올리면 안전)

**보수적 선택 (안전 마진 확보):**
- 입력: **XT60** (30A 연속, 60A 피크)
- 모터 출력: **XT60 × 2** (각 30A 연속, 보드는 좀 더 큼)
- 또는 입력 XT60 + 출력 XT30 (마진 작음, 비용·사이즈 절약)

#### 보호회로 (15A+ 연속 대응 강화)

| 부품 | 사양 | 이유 |
|---|---|---|
| **eFuse 컨트롤러** | **LTC4368-1** (0~80V, surge stopper) | 4-in-1 통합 (§3-1A 상세) |
| eFuse 외부 N-MOSFET | **AUIRFS8409-7P** (40V/240A) 또는 **IPB180N06S4-02** (60V/180A) | 15A+ 통과, 저 RDS(on) |
| 역전압 보호 | LTC4368 내장 ideal diode 모드 | MOSFET 한 개로 통합 |
| 퓨즈 (백업) | **자동차용 ATM blade 30A 교체형** | eFuse 고장 시 2차 안전망 |
| 입력 TVS | **SMCJ58A** (1500W, 58V 클램프) | 인덕티브 킥백 |
| 입력 벌크 캡 | **470µF/63V Al-polymer × 4** + 22µF/63V X7R × 4 | 모터 회생·inrush·dV/dt |
| Common-mode choke | **Wurth 744226601 (60A 정격)** | EMI |
| Y-cap (CM 노이즈) | 1nF 1kV ceramic × 2 | 라인-GND 노이즈 |
| **배터리 모니터링** | **INA228 + 1mΩ shunt (필수)** | 전압·전류·전력 + SoC 추정 (§3-1B 참조) |

#### 모터 전원 트레이스/PCB

| 항목 | 사양 |
|---|---|
| 외층 동박 | **2 oz/ft²** (1oz의 2배 두께) |
| 입력 트레이스 (30A) | **폴리곤 채움 + 200mil 너비 보장** |
| 모터 출력 트레이스 (15A) | **폴리곤 + 100mil 너비** |
| Via | 0.5mm 드릴, 한 라인에 8개 이상 (전류 분산) |
| 모터 폴리곤 격리 | 신호 영역에서 3mm 이상 떨어뜨림 |
| GND | 단일점(star) 연결, 모터 GND ↔ 로직 GND |

#### 로직 전원 (모터 라인에서 ferrite 분기)

- TPS62933 (2.5MHz, 60V, 2A) — 모터 전원에서 분기, 입력 캡 별도 ferrite
- TLV75533 × 2 (디지털/아날로그)
- 디커플링: 100nF × MCU 핀 수, 4.7µF × VDD 도메인 그룹

#### 사이즈 영향 (15A 대응 추가)

- XT60 입력 + XT30 출력 ×2 = 95 + 60×2 = **215 mm²** (XT30 통일이면 180mm²)
- eFuse + 외부 MOSFET + 큰 캡 ×4 + 큰 choke = **~200 mm²**
- 30A 트레이스 폴리곤 영역 (라우팅 마진) = **+100 mm²**
- **총 추가: ~515 mm² → 보드 55×55 → 60×60mm 권장 (4-layer)**

또는 6-layer 양면 실장으로 55×55mm 유지 가능.

---

### 3-1B. 배터리 잔량 모니터링 (사용자 요청)

#### 우리 역할: 잔량 표시 + 저전압 경고 (BMS는 배터리 팩 내장)

> **참고:** 본격 BMS(셀 밸런싱·과충전 차단)는 배터리 팩 자체에 있어야 함.
> 우리 보드는 **외부에서 본 잔량(SoC) 추정 + 저전압 자동 차단** 만 담당.

#### 측정 항목

| 항목 | 방법 | 부품 |
|---|---|---|
| 팩 전압 | INA228 bus voltage 입력 | INA228 내장 ADC (16-bit, 0~85V) |
| 팩 전류 (charge/discharge) | 1mΩ shunt + INA228 차동 입력 | INA228 + Susumu shunt 2512 |
| 팩 전력 (W) | INA228 자동 계산 | — |
| 누적 전하 (mAh) | INA228 ENERGY/CHARGE 레지스터 (자체 적분) | INA228 내장 |
| SoC (잔량 %) | 펌웨어에서 OCV 곡선 매칭 + 쿨롱 카운팅 | MCU 펌웨어 |
| 팩 온도 | NTC (외부 커넥터로 배터리에 부착) | NTC 10k + JST-GH |

#### INA228 vs 다른 옵션

| IC | Vcm max | 정밀도 | 인터페이스 | 우리 적합성 |
|---|---|---|---|---|
| **INA228** ⭐ | **85V** | 16-bit, 1.5mV BUS | I²C | 24/36/48V 모두 OK |
| INA226 | 36V | 16-bit | I²C | 36V 부족, 48V 불가 |
| INA229 | 85V | 16-bit | SPI | I²C 핀 부족 시 대안 |
| MAX17320 | 정밀 fuel gauge | 셀별 측정 가능 | I²C | BMS 기능까지, 과잉 |
| BQ40Z80 | full BMS | 셀별 + 충전제어 | SMBus | 우리는 BMS 안 함 |

→ **INA228 확정** — 24~48V 전 범위 커버, 가격 ~$3.

#### 펌웨어 SoC 추정 알고리즘

```
1. 부팅 시: 무부하 OCV(개방회로전압) 측정 → Li-ion OCV-SoC 곡선에서 초기 SoC 추정
   예: 4S Li-ion 16.8V = 100%, 14.8V = 50%, 12.8V = 0%
2. 운용 중: INA228 CHARGE 레지스터(쿨롱 카운팅)으로 SoC 적분 갱신
   SoC[t] = SoC[t-1] - (I × Δt) / 배터리 용량
3. 주기적 (예: 충전 직후) OCV 재측정으로 보정
4. 저전압 경고: SoC < 20% → RGB LED 황색, ESP32로 알림
5. 저전압 차단: SoC < 5% 또는 V < UVLO → motor enable 자동 해제 + 안전 모드
```

#### 배터리 정보 활용처

| 활용 | 채널 |
|---|---|
| 보드 RGB LED | 색상 (녹/황/적) + 깜빡임 빈도 |
| ESP32 BLE → 폰 앱 | 실시간 % + 예상 사용 시간 |
| SD 카드 로깅 | 시계열 SoC, 전류, 전력 |
| Jetson UART 텔레메트리 | 실시간 % (HUD/UI 표시) |
| 자동 안전 동작 | 저전압 시 motor disable |

#### 추가 부품 (배터리 모니터링)

| 부품 | 사양 | 위치 |
|---|---|---|
| INA228IDGSR | VSSOP-10, I²C | 입력 라인 (eFuse 출력 단) |
| Shunt resistor | 1mΩ 2W 2512 (Susumu CSS2H) | 입력 +V 라인 직렬 |
| 배터리 NTC 입력 | JST-GH 2pin + 10kΩ 풀업 | MCU ADC 1채널 |
| SoC 표시 LED | (RGB 재활용) | 별도 부품 X |

면적 추가: ~40 mm² (INA228 + shunt + NTC 커넥터)

---

### 3-1A. eFuse (LTC4368) 상세 설명 (사용자 질문)

#### eFuse가 뭔가
"electronic fuse" = 반도체 IC로 만든 통합 보호 장치.
일반 퓨즈 + 다이오드 + TVS + 비교기 + soft-start 회로를 **단일 IC + 외부 MOSFET 1개**로 통합.

#### LTC4368이 한꺼번에 해주는 것

| 기능 | 일반 회로로 만들면 | LTC4368은 |
|---|---|---|
| **OVP (Over-Voltage Protection)** | 비교기 + Zener + MOSFET driver = 부품 5~7개 | 저항 2개로 임계값 설정, 자동 차단 |
| **UVLO (Under-Voltage Lockout)** | 비교기 + 히스테리시스 회로 | 저항 2개로 설정 |
| **RVP (Reverse Voltage)** | P-MOSFET ideal diode 회로 | 같은 외부 MOSFET 재활용 |
| **Inrush Current Limit** | NTC + soft-start 회로 | 자동, 캡 1개로 ramp 시간 설정 |
| **Surge stopper** | 큰 TVS + crowbar | OVP가 입력 전압 자른 뒤 출력 유지 |
| **Power Good 신호** | 별도 비교기 IC | 핀 1개 → MCU EXTI |
| **Fault latch** | RS 래치 회로 | 핀 설정으로 latch/auto-retry 선택 |
| **외부 MOSFET 게이트 구동** | 별도 driver IC | 내장 charge pump |

#### 모터 보드에서 특히 중요한 이유

1. **회생 전류 (Motor regen)** — Elmo 드라이버가 감속 시 운동에너지를 전기로 되돌림 → 입력 전압이 24V → 30V+ 로 솟음. eFuse OVP가 28V에서 자르면 MCU·캡 등 모든 저전압 부품 보호.

2. **케이블 단락 (short)** — 모터 케이블이 우연히 GND에 닿으면 일반 퓨즈는 ms 단위 응답이라 **수백 A까지 흐른 뒤** 끊김. eFuse는 **μs 단위 즉시 차단** → 트레이스·MOSFET 보호.

3. **핫 플러그 (hot-swap)** — 24V 큰 벌크캡이 비어있는 상태에서 전원 갑자기 꽂으면 inrush 전류 100A+ 가능. 일반 퓨즈도 끊기고 캡 손상. eFuse는 **0V→24V를 천천히 ramp** (예: 10ms) → 부드럽게 충전.

4. **역접속 (가장 흔한 사고)** — 24V 라인 거꾸로 꽂는 실수 한 번에 모든 부품 사망 (수백만원). LTC4368이 외부 N-MOSFET을 ideal diode로 동작시켜 **역방향 0A**.

5. **과전류 (over-current)** — 모터 stall 또는 short 시 정상 30A 넘어가면 **shunt 저항 측정 → 즉시 차단** (μs 단위). 일반 퓨즈로는 불가능한 속도.

6. **부저전압 lockout** — 배터리 방전돼서 18V 이하 떨어지면 자동 OFF (배터리 보호 + MCU 비정상 동작 방지).

#### 비교: eFuse vs 개별 부품

| 항목 | 개별 부품 | LTC4368 eFuse |
|---|---|---|
| 부품 수 | 7~10개 | 2개 (IC + MOSFET) + 저항 5개 |
| 면적 | ~200 mm² | ~50 mm² |
| 임계값 튜닝 | 하드웨어 변경 필요 | 저항만 교체 |
| 응답 속도 | 퓨즈 ms / 비교기 μs | μs 단위 통합 |
| 진단 (MCU 알림) | 별도 회로 | Power Good 핀 1개 |
| 비용 | $5~10 (부품 합) | LTC4368 ~$6 + MOSFET $2 |
| 신뢰성 | 다 자기 따로 | 단일 IC, 검증된 IP |

#### 비유
- **일반 퓨즈** = 한 번 끊기면 갈아 끼워야 함, 응답 느림 (ms)
- **다이오드** = 역접속만 막음, 항상 0.4V 손실
- **eFuse** = 가전제품 멀티탭의 surge protector + GFCI + breaker + soft-start가 다 합쳐진 IC. 자동 복구·MCU 알림 기능까지.

#### 추천 부품 (15A+ 모터 보드용)

| 부품 | 사양 | 가격 | 비고 |
|---|---|---|---|
| **LTC4368-1** | 0~80V, MSOP-12 | ~$6 | latch fault (한 번 차단 시 reset 필요) |
| LTC4368-2 | 0~80V, MSOP-12 | ~$6 | auto-retry (자동 복구) |
| LM5069 | 9~80V, hot-swap controller | ~$5 | 더 강력하지만 복잡 |
| TPS25940 | 4~18V | ~$2 | 우리 전압엔 부족 (24V↑) |
| MAX17608 | 4.5~60V | ~$3 | 60A 한계, 우리 60V 마진 부족 |

**선택: LTC4368-1** (latch 모드 — 누가 OVP/OCP 걸렸는지 명확히 알 수 있음, MCU에서 reset)

**외부 MOSFET (15A+ 연속 통과):**
- AUIRFS8409-7P: 40V/240A, RDS(on) 1.4mΩ — 매우 여유
- IPB180N06S4-02: 60V/180A, RDS(on) 2.7mΩ — 24V/48V 모두 OK
- AOTL66520: 60V/100A — 컴팩트

#### 결론
eFuse는 모터 보드에서 **선택이 아닌 필수**에 가까움. 추가 비용 ~$8/board, 면적 ~50mm², 보호 기능 통합으로 부품 수 절반.
**TPS25940 → LTC4368-1로 변경** (24~48V 대응).

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

## 4-11. PCB 스택업 / 제조 사양 (v2.7 — 6L 컴팩트)

> 사용자 요청: "layer ↑로 사이즈 ↓ + 성능·안전" → 4L → **6L 양면 실장** 전환.

### 추천 스택업 (50×50mm 6-layer)

```
Layer 1 (TOP)     — 신호 + 부품 (MCU, ESP32, microSD, 커넥터)   1 oz + 0.5oz plating = 2oz
Layer 2 (GND1)    — 솔리드 GND 평면 (return path, EMI shield)     0.5 oz
Layer 3 (SIG/PWR) — 모터 전원 폴리곤 + 디지털 신호 (격리)         1 oz
Layer 4 (PWR)     — 3.3V/5V 평면 (분산 디커플링)                   1 oz
Layer 5 (GND2)    — 솔리드 GND 평면 (analog return)                 0.5 oz
Layer 6 (BOT)     — 신호 + 패시브 (디커플링 캡, 풀업, BGA-like)     1 oz + plating = 2oz
```

**핵심 설계 룰:**
- **신호 → GND → 신호 → 전원 → GND → 신호** = 모든 신호층이 GND 평면에 인접 (impedance control + EMI ↓)
- **외층 2oz**: 30A 입력 트레이스 + 모터 출력 트레이스
- **내층 1oz**: 신호 + 전원 분산
- **GND 평면 2개**: 모터 GND ↔ 로직 GND 단일점 연결, 양쪽 다 솔리드 → impedance 매우 낮음
- **3.3V/5V 평면**: MCU 디커플링 효과 극대화 (별도 캡 없이도 평면 캐패시턴스 활용)

### 8-layer 옵션 (더 컴팩트, 모터 전원 분산 best)

```
L1 TOP    — 신호 + 부품
L2 GND    — 솔리드 GND
L3 SIG    — 디지털 신호 라우팅
L4 PWR    — 모터 전원 폴리곤 1
L5 PWR    — 모터 전원 폴리곤 2 (병렬 → 30A 더 분산)
L6 SIG    — 아날로그 신호 (loadcell, current sense)
L7 GND    — 솔리드 GND (analog return)
L8 BOT    — 신호 + 패시브
```
8L 장점: 모터 전원 PWR 평면 2개 병렬 → 동일 전류에 면적 절반, 발열 분산.

### 제조 사양 (6L 추천 기준)

| 항목 | 사양 | 비고 |
|---|---|---|
| 보드 두께 | 1.6 mm 표준 (1.0mm 옵션) | 두께 ↓ → 캡 가까이 → 노이즈 ↓ |
| 최소 트레이스/간격 | **4 mil / 4 mil** | 6L 컴팩트 라우팅 |
| 최소 비아 | 0.2mm 드릴 / 0.45mm 패드 | 일반 6L 가능 |
| Via-in-pad | **사용 (BGA-like 부품 + 디커플링)** | epoxy fill + cap plating |
| 외층 동박 | **2 oz/ft²** (1+1 plating) | 30A 모터 트레이스 |
| 내층 동박 | 1 oz/ft² | 신호·전원 분산 |
| 표면 처리 | **ENIG** (Electroless Ni/Au) | 모터 고전류 패드 산화 방지, 미세 피치 SMT 정확도 |
| 솔더 마스크 | LPI 양면 | 표준 |
| 실크 | 양면 | 부품 식별 |

### 성능 / 안전 / EMI 이득 (4L → 6L 전환)

| 항목 | 4L | 6L | 8L |
|---|---|---|---|
| 신호-GND 인접 | 일부만 | **전부** | 전부 |
| Crosstalk | 보통 | **낮음 (-10dB)** | 매우 낮음 |
| ADS131M04 SNR | 기준 | **+6~10 dB** (loadcell 정밀도 ↑) | +10 dB |
| USB HS eye 마진 | 빠듯 | **여유** | 매우 여유 |
| 모터 전원 임피던스 | 1× | 0.5× | 0.25× |
| EMI 방사 | FCC 한계 위태로울 수 |  **확실히 통과** | 통과 |
| ESD 견디기 | 보통 | 강함 | 매우 강함 |
| Thermal | 외층 2oz 의존 | 내층 PWR로 분산 | 더 분산 |

### PCB 제조사 견적 (참고, 5장 시제품)

| 제조사 | 4L 60×60 | 6L 50×50 | 8L 45×45 |
|---|---|---|---|
| JLCPCB | $15 | $45 | $90 |
| PCBWay | $25 | $55 | $110 |
| OSH Park | $50 | $120 | $200 |

**총비용 영향 (5장):** 6L = 4L 대비 +$150 정도. 보드 1장 만들 때 무시할 수준.

---

## 4-10. 사이즈 / 폼팩터 예산 (모터 15A+ 반영, v2.7 — 6L 양면 실장)

| 블록 | 면적 (대략) | 비고 |
|---|---:|---|
| MCU H723VGT6 LQFP100 (14×14) | 196 mm² | 확정 |
| ESP32-C3-MINI-1**U** | 219 mm² | + 안테나 keep-out (IPEX는 작게 OK) |
| microSD push-push | 210 mm² | 필수 |
| ADS131M04 TQFP-32 + REF | 80 mm² | |
| **DC 입력 XT60 + 출력 XT30 ×2** | **215 mm²** | 모터 30A 입력, 15A 출력 |
| **모터 전원 보호회로 (LTC4368 eFuse + 외부 MOSFET + 470µF×4 + choke)** | **~200 mm²** | 15A+ 대응 |
| **30A 트레이스 폴리곤 마진** | **~100 mm²** | 라우팅 여유 |
| Buck (TPS62933 + 인덕터·캡) | 100 mm² | 2.5MHz |
| LDO x2 + 캡 | 50 mm² | |
| USB-C 커넥터 | 80 mm² | DFU·디버그 |
| CAN 트랜시버 + ESD | 30 mm² | |
| 엔코더 IC AS5048A x2 | 60 mm² | |
| 인터커넥트 (JST-GH x6: CAN, Jetson UART, SYNC, EBIMU, Loadcell ×2, Estop) | ~180 mm² | |
| **E-stop 회로 (AND gate, latch, debounce)** | **~25 mm²** | |
| 디커플링 캡, 패시브 | ~300 mm² | 양면 분산 |
| **부품 합계** | **~2150 mm²** | (15A+ 대응 추가) |
| 양면 실장 시 한 면당 | **~1100 mm²** | 디커플링·풀업 등 패시브 BOT |
| × 1.3 (6L 라우팅 마진, 양면) | **~1430 mm²** | 한 면 기준 |
| **2면 활용 시 보드 면적** | **~2200 mm²** | 50×50 = 2500mm² 안에 들어감 |

**v2.7 수정 — 사용자 요청: layer ↑로 사이즈 ↓ + 성능·안전:**

| 옵션 | 사이즈 | Layer | 면적 | PCB 단가 (5장 시제품) | 평가 |
|---|---|---|---:|---|---|
| ~~4L 큰 보드~~ | ~~60×60~~ | ~~4L~~ | ~~3600 mm²~~ | ~~$15/장~~ | wearable 부적합 |
| **6L 컴팩트** ⭐ | **50×50** | **6L 양면 실장** | **2500 mm²** | ~$45/장 | 추천 — 성능·사이즈·비용 균형 |
| 8L 극컴팩트 | 45×45 | 8L 양면 실장 | 2025 mm² | ~$70/장 | 모터 전원 분산 best |
| 8L HDI | 40×40 | 8L HDI + microvia | 1600 mm² | ~$120/장 | 양산엔 OK, 시제품 과잉 |

**추천: 50×50mm 6-layer** (또는 45×45mm 8L 더 줄이고 싶으면)

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

## 6. 결정 현황 (v2.7)

### 이번 라운드 확정 (사용자 "layer↑로 사이즈↓ + 성능·안전")

| 항목 | 결정 | 근거 |
|---|---|---|
| Layer | **6-layer 양면 실장** (또는 8L 더 줄이려면) | 사용자 요청 |
| 보드 사이즈 | **50 × 50 mm** (또는 8L면 45 × 45) | wearable 컴팩트 |
| 외층 동박 | 2 oz (30A 모터 트레이스) | 안전 |
| 내층 동박 | 1 oz | 신호·전원 분산 |
| 표면 처리 | ENIG | 정밀도·산화 방지 |
| 스택업 | SIG/GND/SIG/PWR/GND/SIG | 모든 신호 GND 인접 → SI/EMI 이득 |
| 모터 출력 커넥터 | **XT60 ×2** (안전 마진) | 사이즈 절약된 만큼 안전성 ↑ |

### 남은 결정

1. **6L (50×50) vs 8L (45×45)** — 8L은 +$25/장, 모터 전원 평면 2개로 발열 분산 best. wearable 5mm 차이가 크면 8L.
2. **CubeMX 칩** — H723VGT6로 진행 OK?
3. ~~INA226 입력 전류 모니터 포함?~~ → **INA228로 확정 포함** (사용자: "배터리 잔량도 확인해야지")
4. **모터 모델·전압·max 전류** (Phase 5)

---

## 6. 모터 미확정 상태에서 진행 가능한 것

- 전원 입력 단(보호회로): 60V 입력 가능한 부품으로 24/36/48V 모두 커버
- CAN-FD 트랜시버: Elmo 모델 무관 동일
- 모터 전원 enable: 외부 SSR/컨택터 트리거 신호만 핀 할당
- Estop, 워치독, 보호 회로
- MCU/USB/SD/IMU 입력/로드셀/엔코더: 모터와 무관 → **전부 확정 가능**

→ Phase 5 외 모든 단계가 선행 가능.
