---
aliases: [Loadcell Amp Block]
tags: [type/block, phase/A, domain/sensor, priority/high]
type: schematic_block
phase: A
status: pending
kicad_sheet: loadcell_amp.kicad_sch
---

# Loadcell Amp

> **사용자 우선 블록** — "앰프 미리 만들어놓기" 요청

## 목적

케이블 텐션 로드셀 → 24-bit ADC → SPI/DMA → MCU.
1 kHz force feedback 제어 루프의 입력.

## 핵심 부품

- [[ADS131M04]] — 24-bit ΔΣ, 32 kSPS, 4ch 동시 샘플
- [[REF5025]] — 2.5 V 정밀 레퍼런스
- 3.3V analog rail ([[Logic Power]] ferrite 분리)
- JST-GH 4pin × 2 (로드셀 커넥터)
- 안티앨리어싱 RC × 4채널

## 관련 개념

- [[Loadcell Amplifier Design]] — 설계 원칙 전체

## 핀맵 (MCU 측)

| 신호 | MCU 핀 | 비고 |
|---|---|---|
| SPI1_SCK | PA5 | |
| SPI1_MISO | PA6 (DOUT) | |
| SPI1_MOSI | PA7 (DIN) | |
| /CS | PA4 | |
| /DRDY | PC0 (EXTI) | 데이터 준비 인터럽트 |
| /SYNC_RESET | PC1 | 동기 시작 |

## 외부 회로

### 차동 입력 안티앨리어싱 (채널 1, 2)

```
S+ ──[R 1k]──┬──── AIN_nP
             │
           [C 10nF] (차동)
             │
S- ──[R 1k]──┴──── AIN_nN
             │
           [C 100pF × 2] (CM to GND)
```

### REF5025 레퍼런스

```
3V3_A ─[100nF + 10µF]─ VIN ── OUT(2.5V) ─[4.7µF + 100nF]─► ADS131M04 REFIN
                               NR ─[1µF]─ GND
```

### 로드셀 Excitation

- 옵션 A: 3.3V_D → E+ / GND → E- (간단)
- **옵션 B ⭐**: 별도 3.3V_A → E+ (더 정밀)

## 커넥터 (로드셀 × 2)

JST-GH 4pin: `E+ / S+ / S- / E-`

## 데이터시트 / 레퍼런스

- [ ] TI ADS131M04 데이터시트
- [ ] ADS131M04EVM schematic
- [ ] TI SBAA532 design considerations
- [ ] TI REF5025 데이터시트

## 작업 체크리스트

- [ ] reference schematic 수집
- [ ] KiCad 심볼 확인 / 생성
- [ ] TQFP-32 + SO-8 풋프린트
- [ ] 0402 패시브 배치
- [ ] sheet 그리기 (Phase A 우선)
- [ ] ERC 통과
- [ ] peer review
- [ ] CubeMX `.ioc` SPI1/EXTI 핀 대조
