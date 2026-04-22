---
aliases: [Motor Power Block]
tags: [type/block, phase/B, domain/power, status/deferred]
type: schematic_block
phase: B
status: deferred
kicad_sheet: motor_power.kicad_sch
---

# Motor Power

> Phase B — 모터 전류 계산·전압 확정 후 구체 부품 정격 확정. 토폴로지와 보호 부품군은 **legacy review 검증 완료** 이므로 지금 freeze 가능.

## 목적

배터리 → 보호 → 모터 분배 + 로직 전원 분기. 이전 보드 파손 원인 (재생에너지·GND bounce·인러시) 모두 해결.

## 전체 토폴로지 (24V 기준, legacy 확정본)

```
배터리 24V LiPo (완충 25.2V) + XT60 입력
         │
    ┌────▼──────────────────────────────────┐
    │  [DMP2305U P-MOS] + [BZT52C12 12V     │   역전압 보호
    │   제너 + R 100kΩ] 역전압 보호        │   (VGS ±20V 이내)
    └─────────────┬──────────────────────────┘
                  │
    ┌─────────────▼──────────────────────────┐
    │  [SL08-2R005 NTC × 2 직렬 = 4Ω]      │   인러시 제한
    │  [Omron G5V-1 릴레이 bypass,           │   (6A 피크, τ=45ms)
    │   100ms 후 자동 단락]                   │   → RC 지연
    └─────────────┬──────────────────────────┘
                  │
    24V_MOTOR 버스 ─┤
                  │
        ┌─────────┼─────────┐───────────┐
        │         │         │           │
   [BUS_BULK]  [BUS_TVS]   [Brake]    [Battery Mon]
   Rubycon    P6KE33CA×2   TLV3201     [[INA228]] ×2
   35ZLH      37.8V        26.5V 임계   1mΩ shunt
   4700µF×2   클램프        IPB025N10N3
   = 9400µF                → 3Ω/50W
                             → GND
                             ([[Brake Resistor Circuit]])
                  │
      ┌───────────┼────────────┬─────────────┬───────────┐
     모터1      모터2        모터3         모터4
  ─[SMAJ28CA]── ─[SMAJ28CA]── ─[SMAJ28CA]── ─[SMAJ28CA]── 로컬 TVS
  ─[470µF/35V] ─[470µF/35V]  ─[470µF/35V] ─[470µF/35V]  OS-CON
  ─[BLM31AG]   ─[BLM31AG]    ─[BLM31AG]   ─[BLM31AG]    페라이트
  ─[커넥터]    ─[커넥터]     ─[커넥터]    ─[커넥터]

                  │
                  └──── 5A 퓨즈 ─── 24V_LOGIC 분기
                                    [10µH + 100µF 필터]
                                    [AP63205WU-7 또는 TPS62933 → 5V]
                                    → [[Logic Power]]

PGND ──[BLM21PG601 단 1개]── AGND (스타 그라운드)
```

## 핵심 부품 (24V 기준)

### 역전압 보호

- [[DMP2305U]] P-MOS SOT-23 (V_DSS -30V)
- [[BZT52C12]] 12V 제너 (VGS 클램프)
- R 100kΩ 풀업

### 인러시 제한

- [[SL08-2R005]] NTC × 2 직렬 (4Ω)
- Omron G5V-1-DC5 릴레이 (5V DC, 10A 접점)
- 릴레이 구동: 2N7002 MOSFET + 1N4148 플라이백 + RC (10k + 10µF → ~100ms 지연)

### 버스 보호

- **BUS bulk**: Rubycon 35ZLH4700M × 2 (4700µF/35V, 병렬 → 9,400µF)
- **BUS TVS**: [[P6KE33CA]] × 2 병렬 (37.8V 클램프, 1200W)
- **BUS 전류 센싱**: Susumu CSS2H 1mΩ 2W (입력 직렬) → [[INA228]] × 2 입력

### 브레이크 회로 (재생 에너지 소산)

→ [[Brake Resistor Circuit]] 상세

- [[TLV3201]] 비교기 (26.5V 임계)
- 분압 100kΩ + 10kΩ
- [[IPB025N10N3]] 브레이크 MOSFET
- Arcol HS50 3R0 3Ω/50W 저항

### 모터별 로컬 (× 4)

- [[SMAJ28CA]] TVS (28V 스탠드오프)
- Panasonic SVPD471M35 OS-CON 470µF/35V
- Murata BLM31AG601SN1L 페라이트 600Ω@100MHz/3A
- 커넥터 (XT30 또는 XT60 — Phase B 모터 전류 확정 후)

### 로직 전원 분기

- 5A 퓨즈 (모터 전원 단락 시 로직 차단)
- 10µH 필터 + 100µF
- [[AP63205WU]] 또는 [[TPS62933]] → 5V (24V 면 AP63205, 48V 면 TPS62933)
- → [[Logic Power]] 블록으로 이어짐

### PGND ↔ AGND star ground

- Murata BLM21PG601SN1D 600Ω@100MHz 페라이트 **단 1개 연결점**

## ⚠️ 48V 시스템으로 갈 때 (Phase B 최종 결정)

legacy 는 24V 기준. 48V (24V × 2 직렬) 로 가면 재선정 필수 부품:

| 부품 | 24V 값 | 48V 값 (예상) |
|---|---|---|
| 벌크 캡 V 정격 | 35V | **63V** (Rubycon 35ZLH → 63ZLH) |
| 모터 TVS 스탠드오프 | 28V | **56V** (SMAJ56CA) |
| 버스 TVS | P6KE33CA | **P6KE68CA** |
| 브레이크 임계 | 26.5V | **55V** (분압 재계산) |
| 브레이크 MOSFET V_DSS | 100V | 100V 충분 (마진) |
| 역전압 P-MOS V_DSS | -30V | **-60V** (DMP2305 → 다른 P-MOS) |
| 역전압 제너 | 12V | 12V 동일 (VGS 기준) |
| 버크 컨버터 | AP63205WU (32V max) | **TPS62933** (60V) |

→ [[Open Questions]] 4번 "배터리 최종 전압" 결정 후 매트릭스 [[Voltage-Current Scaling]] 참조.

## 핀맵 (MCU 측 — 제어 신호)

| 신호 | MCU 핀 | 방향 | 용도 |
|---|---|---|---|
| BRAKE_ACTIVE_LED | PE6 | 출력 | 브레이크 동작 중 LED |
| BATT_CURRENT_ALERT | PE5 | 입력 EXTI | INA228 과전류 알림 |
| RELAY_BYPASS | 자동 (RC) | - | MCU 개입 없음 |
| MOTOR_ENABLE | PE3 | 출력 | [[Safety UI]] AND gate → 모터 드라이버 |

(주의: MCU 는 eFuse 직접 제어 안 함 — 보호 회로가 자율. MCU 는 상태 모니터링만.)

## 관련 개념

- [[Regen Energy Protection]] ⭐
- [[GND Bounce Protection]]
- [[Isolated CAN]]
- [[Brake Resistor Circuit]]
- [[Inrush Current Limiting]]
- [[Motor Power Isolation]] — PCB 레이아웃 규칙
- [[Voltage-Current Scaling]] — 전압별 부품 매트릭스

## 관련 부품

- [[DMP2305U]], [[BZT52C12]], [[SL08-2R005]]
- [[P6KE33CA]], [[SMAJ28CA]]
- [[TLV3201]], [[IPB025N10N3]]
- [[AP63205WU]], [[TPS62933]]
- [[INA228]]
- [[LTC4368-1]] — 내 원안 eFuse, legacy 선택 아님 (rejected)
- [[IPB180N06S4]] — LTC4368 짝 MOSFET, 동일 (rejected)

## 데이터시트

- [ ] Diodes Inc DMP2305U-7
- [ ] Diodes Inc AP63205WU-7
- [ ] Vishay P6KE33CA, SMAJ28CA
- [ ] TI TLV3201
- [ ] Infineon IPB025N10N3
- [ ] Ametherm SL08-2R005
- [ ] Omron G5V-1 relay
- [ ] Rubycon 35ZLH4700M
- [ ] Panasonic SVPD471M35 OS-CON

## 작업 체크리스트 (Phase B 시작 시)

- [ ] [[Open Questions]] 1번 — 필요 연속 전류 계산 완료
- [ ] [[Open Questions]] 2번 — 모터 모델 확정
- [ ] [[Open Questions]] 3번 — Elmo 드라이버 확정
- [ ] [[Open Questions]] 4번 — 배터리 24V vs 48V 확정
- [ ] 위 결정 따라 48V 매트릭스 적용 or 24V legacy 부품 유지
- [ ] 벅 컨버터 AP63205WU (24V) vs TPS62933 (48V 대응) 선택
- [ ] 커넥터 XT30/XT60/XT90 선정
- [ ] 브레이크 임계값 재계산 (전압 확정 후)
- [ ] 회로도 그리기
- [ ] ERC + 검증 (4-agent 스타일 체크리스트 legacy 따라)
