---
aliases: [Block Diagram, System Architecture]
tags: [project, architecture]
---

# Block Diagram

## 시스템 구성도 (v3.11, AS5048A 제거)

```
┌────────────────────────────────────────────────────────────────┐
│                 Jetson Orin NX 16GB (157 TOPS)                  │
│                        │            │                          │
│                   [PCIe ZED Link]   │                          │
│                        │            │                          │
│                  [ZED X Mini]       │                          │
│                  (GMSL2, IP67,      │ UART + HW SYNC            │
│                   120fps, 150g)     │ (1 Mbps DMA, 시계 정합)   │
│                                     │                          │
└─────────────────────────────────────┼──────────────────────────┘
                                      │
                 (HW SYNC GPIO — 공유 시계 이벤트, 사용자 확정 v3.10)
                                      │
┌─────────────────────────────────────▼──────────────────────────┐
│     STM32H743VIT6 (480 MHz Cortex-M7, 2MB Flash, 1MB RAM)       │
│                                                                  │
│  FDCAN1 ─► ISO1050 (격리 5000Vrms) ─► Elmo 드라이버             │
│            + MGJ2D05050SC 격리 DC/DC                             │
│                                                                  │
│  UART2   ─► Jetson          UART3 ─► ESP32-C3-MINI-1U (BLE)     │
│  UART4   ─► EBIMU 수신기    SYNC  ─► 양방향 GPIO                │
│                                                                  │
│  SPI1 ─► ADS131M04 (24-bit, 32kSPS, 4ch, 로드셀 × 2)            │
│         + REF5025 2.5V                                           │
│  │                                                                  │
│  I²C1 ─► INA228 × 2 (배터리 팩 × 2 모니터)                      │
│                                                                  │
│  SDMMC1 ─► microSD (4-bit, 로깅)                                │
│  USB FS ─► USB-C (디버그·DFU)                                   │
│                                                                  │
│  GPIO EXTI ─► E-stop NC 접점                                    │
│  GPIO → 74LVC1G08 AND → Motor Enable (HW + SW 이중)             │
└──────────────────────────────────────────────────────────────────┘
                                      │
                  ┌───────────────────┼───────────────────┐
                  │                   │                   │
   ┌──────────────▼──────┐ ┌──────────▼──────────┐ ┌─────▼──────┐
   │ ESP32-C3-MINI-1U   │ │ Sensors (3.3V)      │ │ microSD    │
   │ BLE 5.0 + WiFi     │ │  로드셀 × 2 (JST-GH)│ │ (SDMMC)    │
   │ IPEX 외부 안테나   │ │  인코더 × 2          │ └────────────┘
   │ UART to MCU        │ │  EBIMU 수신기 (5V)   │
   └────────────────────┘ └──────────────────────┘

╔══════════════════════════════════════════════════════════════╗
║           모터 전원부 (legacy 보호 아키텍처 채택)              ║
╠══════════════════════════════════════════════════════════════╣
║                                                                ║
║  6S Li-ion 25.2V 완충 ──► XT60 ──► DMP2305U + BZT52C12         ║
║  (24V 공칭)                       (역전압, VGS ±20V 보호)      ║
║                              │                                 ║
║                  ┌───────────▼────────────┐                    ║
║                  │ SL08-2R005 × 2 직렬    │  인러시 제한        ║
║                  │ + Omron G5V-1 bypass  │  (NTC 4Ω, 100ms)   ║
║                  └───────────┬────────────┘                    ║
║                              │                                 ║
║     24V_MOTOR 버스 ──────────┤                                 ║
║                              │                                 ║
║    ┌────────┬────────────┬──▼────────┬────────────────┐       ║
║    │        │            │           │                │       ║
║ Bulk×2   P6KE33CA     Brake       INA228 × 2     → 5V Buck   ║
║ 4700µF    × 2          Circuit    1mΩ shunt      (분기)       ║
║ /35V      37.8V        (26.5V     (쿨롱 카운팅)               ║
║           클램프        임계)                                   ║
║           1200W       TLV3201 →                                ║
║                       IPB025N10N3 →                            ║
║                       3Ω/50W                                    ║
║                              │                                 ║
║    ┌────────┬────────────┬──▼────────┬──────────┐             ║
║  Motor1   Motor2     Motor3      Motor4                        ║
║  SMAJ28CA SMAJ28CA   SMAJ28CA    SMAJ28CA        로컬 TVS    ║
║  470µF    470µF      470µF       470µF           OS-CON       ║
║  BLM31AG  BLM31AG    BLM31AG     BLM31AG         페라이트      ║
║  XT60/30  XT60/30    XT60/30     XT60/30         커넥터       ║
║                                                                ║
║  PGND ── [BLM21PG601 단 1개] ── AGND (star ground)             ║
║                                                                ║
╚══════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════╗
║  로직 전원 (PGND ─ferrite─ AGND 분기)                         ║
╠══════════════════════════════════════════════════════════════╣
║  24V ── [페라이트] ── AP63205WU-7 (또는 TPS62933) ── 5V       ║
║                                                   │           ║
║                                       ┌───────────┤           ║
║                                       │           │           ║
║                                  TLV75533-D  TLV75533-A       ║
║                                   (3V3-D)    (3V3-A, ferrite) ║
║                                       │           │           ║
║                                     MCU/IO    ADC/REF         ║
╚══════════════════════════════════════════════════════════════╝
```

## 관련

- [[Goals & Requirements]]
- [[Size Budget]]
- [[Modular Strategy]]
- [[PCB Stackup 6-layer]]
- [[RECONCILIATION]]
- [[_legacy/EXOSUIT_PROTECTION]]

## TODO

- [ ] Excalidraw 블록 다이어그램 (Assets/block-diagram.excalidraw.md)
- [ ] 신호 흐름 / 전원 흐름 별도 다이어그램
- [ ] 격리 경계 시각화 (ISO1050 구역)
