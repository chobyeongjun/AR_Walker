---
aliases: [Motor Power Block]
tags: [type/block, phase/B, domain/power, status/deferred]
type: schematic_block
phase: B
status: deferred
kicad_sheet: motor_power.kicad_sch
---

# Motor Power

> ⚠️ **Phase B — 모터 확정 후 진행.** 현재는 placeholder + 설계 원칙만 정리.

## 목적

DC 입력 → eFuse 보호 → 모터 출력 분배 + 로직 전원 분기.

## 핵심 부품 (후보, Phase B 확정)

- DC 입력 커넥터: XT60 / XT90 / Molex (전류에 따라)
- 모터 출력 커넥터 × 2: XT30 / XT60
- [[LTC4368-1]] eFuse + [[IPB180N06S4]] MOSFET
- 백업 blade 퓨즈 홀더 + blade (15A/30A/40A)
- TVS SMCJ 시리즈 (클램프 전압 V 의존)
- Common-mode choke (전류 의존, Wurth 7443631400 등)
- Y-cap 1 nF 1 kV × 2
- 벌크 캡 Al-poly 470 µF × 4 + X7R 22 µF × 4 (V 정격 Phase B)
- Shunt 1 mΩ ([[Battery Monitor]] 공유)

## 회로 원칙

→ [[Motor Power Isolation]] 참조

```
+V_IN (XT60) ──[퓨즈]─[choke]─[eFuse+MOSFET]─┬─► XT30 Motor 1
                                              │
                                              ├─► XT30 Motor 2
                                              │
                                              ├─► [shunt] ─► INA228 (전류 센싱)
                                              │
                                              └─► [ferrite] ─► [[Logic Power]] Buck 입력
                                              │
                                              └─► 벌크 캡 × N
GND_IN ──────────────────────────────────────► Motor GND + star point
```

## 관련 개념

- [[Motor Power Isolation]] — PCB 레이아웃 원칙
- [[Voltage-Current Scaling]] — 전압별 부품 매트릭스
- [[eFuse]] — LTC4368 설정
- [[E-stop Dual Safety]] — motor_enable_final 신호 배선

## 데이터시트

- [ ] Analog Devices LTC4368
- [ ] Infineon IPB180N06S4

## 작업 체크리스트 (Phase B 시작 시)

- [ ] 모터 모델·전압·전류 [[Open Questions]] 답변
- [ ] 커넥터 선정 ([[Voltage-Current Scaling]] 매트릭스 참조)
- [ ] eFuse OVP/UVLO/OCP 임계 저항 계산
- [ ] 벌크 캡 V 정격 확정
- [ ] TVS 클램프 전압 확정
- [ ] 모터 폴리곤 너비 확정 (2oz 기준)
- [ ] sheet 그리기
- [ ] PCB 레이아웃 시 motor 영역 격리 확인
