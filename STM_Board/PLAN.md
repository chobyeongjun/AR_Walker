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

## 2. MCU 비교·추천 (사용자 요청)

### 후보 비교표

| 후보 | 패키지 | 면적 | CPU/RAM/Flash | USB HS 내장 PHY | Ethernet | 가격대 | 장점 | 단점 |
|---|---|---:|---|---|---|---|---|---|
| **STM32H723VGT6** ⭐ | LQFP100 (14×14) | **196 mm²** | M7 550MHz / 564KB / 1MB | ◯ | ✗ | 중 | **컴팩트 + USB HS 내장 + 충분한 성능**. 자가 납땜 OK | RMII 핀 없음 (Ethernet 불가) |
| STM32H723ZGT6 | LQFP144 (20×20) | 400 mm² | M7 550MHz / 564KB / 1MB | ◯ | ◯ | 중 | Ethernet 가능 | 면적 2배, 보드 사이즈 직격 |
| STM32H723VGH6 | UFBGA100 (7×7) | 49 mm² | M7 550MHz / 564KB / 1MB | ◯ | ✗ | 중 | 가장 작음 | **BGA — 자가 납땜·리워크 사실상 불가, 4-layer 라우팅 어려움** |
| STM32H743VIT6 | LQFP100 (14×14) | 196 mm² | M7 480MHz / **1MB / 2MB** | ✗ (외부 ULPI 필요) | ◯ | 중 | RAM·Flash 더 큼 | **USB HS 위해 외부 PHY 필요 → 부품 추가** |
| STM32H7A3VIT6 | LQFP100 (14×14) | 196 mm² | M7 280MHz / 1.4MB / 2MB | ✗ | ✗ | 중 | RAM 큼 | 클럭 낮음, USB HS 외부 |
| STM32H730VBT6 | LQFP100 (14×14) | 196 mm² | M7 550MHz / 564KB / **128KB Flash** | ◯ | ✗ | 저 | 저렴 | 외부 QSPI Flash 필수 → 부품 추가 |

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

## 6. 아직 남은 결정

1. **사이즈 목표**: (a) 50×50mm 4L (안전, 추천) / (b) 45×40mm 6L (공격적, 비용 ↑)
2. **CubeMX 칩**: 이미 다른 H7로 세팅했어? (없으면 H723VGT6로 진행)
3. **DC 입력 커넥터 전류**: 보드만 전원 받으면 JST-GH (~3A) 충분. 보드를 통해 모터까지 공급할 거면 XT30 필요. → 모터 전원 라인 어떻게 갈래? (보통 보드 우회 권장)
4. **모터/Elmo 모델 + 전압** (Phase 5 보류)

---

## 6. 모터 미확정 상태에서 진행 가능한 것

- 전원 입력 단(보호회로): 60V 입력 가능한 부품으로 24/36/48V 모두 커버
- CAN-FD 트랜시버: Elmo 모델 무관 동일
- 모터 전원 enable: 외부 SSR/컨택터 트리거 신호만 핀 할당
- Estop, 워치독, 보호 회로
- MCU/USB/SD/IMU 입력/로드셀/엔코더: 모터와 무관 → **전부 확정 가능**

→ Phase 5 외 모든 단계가 선행 가능.
