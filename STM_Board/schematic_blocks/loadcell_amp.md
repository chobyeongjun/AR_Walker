# Loadcell Amplifier — ADS131M04 (Phase A · 사용자 우선)

> 24-bit ΔΣ, 32 kSPS, 4채널 동시 샘플링, SPI. 케이블 텐션 force feedback 1kHz 제어 루프용.

## 핀 / 신호

| 핀 | 연결 | 비고 |
|---|---|---|
| AVDD | 3.3V analog (ferrite 격리) | 1µF + 100nF 디커플링 |
| AGND | analog GND | star ground 단일점 |
| DVDD | 3.3V digital | 1µF + 100nF |
| DGND | digital GND | |
| CAP | 1µF X7R → AGND | 내부 LDO 안정화 |
| REFIN | REF5025 출력 (2.5V 정밀) | 4.7µF + 100nF |
| CLKIN | MCU TIM PWM 8.192MHz 또는 내장 osc | 내장이면 미연결 |
| /SYNC_RESET | MCU GPIO | 동기 시작 |
| /CS | MCU GPIO (SPI1 CS1) | |
| SCLK | MCU SPI1 SCK | DMA |
| DIN | MCU SPI1 MOSI | DMA |
| DOUT | MCU SPI1 MISO | DMA |
| /DRDY | MCU EXTI | 데이터 준비 인터럽트 |
| AIN0P / AIN0N | 채널 1 차동 입력 | 케이블 1 로드셀 S+/S- |
| AIN1P / AIN1N | 채널 2 차동 입력 | 케이블 2 로드셀 S+/S- |
| AIN2P / AIN2N | 채널 3 (예비) | |
| AIN3P / AIN3N | 채널 4 (예비) | |

## 외부 회로

### 차동 입력 안티앨리어싱 필터 (각 채널)

```
S+  ──[R 1k]──┬──── AINnP
              │
            [C 10nF] (차동)
              │
S-  ──[R 1k]──┴──── AINnN
              │
            [C 100pF GND] x2 (CM)
```

- 차단 주파수 (차동): ~16 kHz
- 32 kSPS Nyquist 16 kHz 안에 신호 보존 + 노이즈 차단
- ESD: 각 입력에 BAV99 다이오드 → AVDD/AGND 클램프 (옵션)

### 정밀 레퍼런스 (REF5025)

```
3.3V_A ──[100nF + 10µF]── VIN(REF5025) ── VOUT(2.5V) ──[4.7µF + 100nF]──► ADS131M04 REFIN
                                                          │
                                                          └──► 1µF noise reduction cap (NR pin → GND)
```

- 노이즈: 7.5 µVpp (0.1~10Hz)
- 온도 드리프트: 3 ppm/°C

### 로드셀 여기 (Excitation)

- 스트레인 게이지 로드셀: 5V 또는 3.3V 여기 전원
- 옵션 A: 3.3V 디지털 → 로드셀 E+/E- (간단, 약간 노이즈)
- 옵션 B: 별도 정밀 5V 또는 3.3V 여기 전원 (TLV75533 전용)
- → **옵션 B 권장** (force 정밀도 ↑)

## 커넥터 (각 채널 1개)

JST-GH 4pin: E+, S+, S-, E-
- E+/E- = 여기 전원
- S+/S- = 차동 시그널 → 안티앨리어싱 필터

## 데이터시트 / 레퍼런스

- TI ADS131M04 datasheet: app circuits 그대로 차용
- TI ADS131M04 EVM schematic (TIDA-01471 등): 검증된 layout 참고
- REF5025 datasheet
- TI app note SBAA532: "ADS131M0x design considerations"

## ⬜ 작업 항목

- [ ] 데이터시트 PDF `refs/datasheets/` 저장
- [ ] EVM schematic PDF 저장
- [ ] KiCad 심볼 확인 (라이브러리 `Sensor_Strain` 또는 직접 생성)
- [ ] 풋프린트 TQFP-32 + REF SO-8 + 0402 패시브
- [ ] hierarchical sheet 그리기
- [ ] DRC + ERC 통과
- [ ] 핀 할당 CubeMX `.ioc` 와 검증
