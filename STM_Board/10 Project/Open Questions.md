---
aliases: [Open Questions]
tags: [project, status/open]
---

# Open Questions

현재 미결 항목. 결정 시 [[Decisions Log]] 로 이동. v3.5 [[RECONCILIATION]] 반영.

## 🟢 확정됨 (2026-04-20 v3.5 + v3.9)

- [x] MCU 패키지 → [[STM32H743VIT6]]
- [x] 로드셀 ADC → [[ADS131M04]] 유지
- [x] CAN 트랜시버 → [[ISO1050]] 격리
- [x] 배터리 팩 → 6S Li-ion 25.2V (24V 단일 vs 48V 직렬은 Phase B)
- [x] IMU → [[EBIMU24GV6]] + EBRCV24GV6 수신기 (V6, legacy V5 에서 정정)
- [x] 카메라 → ZED X Mini
- [x] Jetson → Orin NX 16GB
- [x] 제어 루프 → 500 Hz
- [x] 모터 제어 모드 → Servo(전류) + SET_POS
- [x] **BLE 포함** → [[ESP32-C3-MINI-1U]] (v3.9 확정)
- [x] **JST-GH 1.25mm 통일** (v3.9 확정, JST 카탈로그 근거)
- [x] **Hardware SYNC GPIO 포함** (v3.10 재추가, 사용자 확정)

## ⏸️ 잠정 (나중에 결정)

- [ ] 보드 사이즈 50×50mm — motor 확정 후 P6 에서 결정
- [ ] ESP32 antenna variant (`-1` PCB vs `-1U` IPEX) — 케이스 설계 시
- [ ] **ZED X Mini 렌즈** (2.2mm 광각 vs 3.8mm 표준) — 2.2mm 권장 (근거리 body tracking)
- [ ] **ZED Link Duo 캡처 카드** 확정 — Orin NX 는 직결 불가, 외부 12~19V 전원 필요
- [ ] **Jetson Orin NX carrier board** 선정 (ZED Box vs Dev Kit vs Seeed vs 기타)

## 🔴 긴급 (모터 결정의 전제)

### 1. 필요 연속 전류 계산 (legacy HANDOFF 1순위)

```
필요 정보:
  τ_assist (목표 보조 토크, Nm)    ← 보행 데이터에서
  d_moment (외골격 모멘트 암, m)   ← 프레임 설계
  r_pulley (풀리 반경, m)          ← 기구 설계

계산:
  F_cable    = τ_assist / d_moment
  τ_motor    = F_cable × r_pulley
  I_required = τ_motor / Kt
```

- [ ] **보행 데이터 (정상인 70kg) 스윙 위상 관절 토크 프로파일**
- [ ] **보조 비율 결정** (10~30% 중 선택)
- [ ] 외골격 프레임 모멘트 암 설계값
- [ ] 풀리 반경 설계값
- [ ] 위 입력으로 I_required 계산 → 모터 후보 중 적합한 Kt 선택

## 🟡 Phase B 확정 (모터 전류 계산 후)

### 2. 모터 모델 (3 후보, legacy 명시)

| 후보 | Kt 범위 | 무게 | 비고 |
|---|---|---|---|
| T-Motor U8 Lite KV85 | 낮은 Kt 높은 속도 | ? | 경량 |
| CubeMars RI60 KV120 | 중간 | ? | 중 토크 |
| Maxon EC-i 40 | 높은 Kt | 무거움 | 고 토크·고가 |

- [ ] 전류 계산 결과 기준 선택
- [ ] 인코더 호환성 ([[AS5048A]] 유무)
- [ ] 무게 제약 확인

### 3. Elmo 드라이버 (3 후보)

| 후보 | 연속 전류 | 조건 |
|---|---|---|
| Elmo Whistle | ≤ 20A | 저전류 모터 시 |
| Elmo Twitter | 중간 | 범용 |
| Elmo Solo Twitter | 중간 | 범용, 고정 속도 |

- [ ] 모터 연속 전류 확정 후 선택
- [ ] CANopen 호환 확인 (CiA 402)

### 4. 배터리 최종 전압

| 옵션 | 전압 | 조건 |
|---|---|---|
| 24V 단일 | 25.2V 완충 | 저속·저토크 충분 시 |
| 48V 직렬 (24V × 2) | 50.4V 완충 | **속도 높여야 할 때** |

- [ ] 모터 KV + 요구 속도 확인
- [ ] 48V 시 모든 보호 부품 V 정격 재검토 ([[Voltage-Current Scaling]])

### 5. 모터 전원 보호 세부 (legacy 설계 있음)

- [ ] [[Brake Resistor Circuit]] 임계 26.5V 가 48V 시스템에도 맞나? (24V 기준, 48V 면 재계산)
- [ ] [[Regen Energy Protection]] 벌크 11,280µF 가 48V 시스템에 충분?
- [ ] 모터별 로컬 SMAJ28CA 클램프 전압 48V 적합?

## 🟢 낮음 (PCB 단계 P6)

- [ ] 6L (50×50) vs 8L (45×45) 최종 확정
- [ ] 단일 보드 vs 로직+파워 2-PCB 분리
- [ ] 모터 출력 커넥터 XT30 vs XT60
- [ ] ISO1050 CAN-FD 업그레이드 (ISO1042) 필요 여부

## 🔵 장기 / 선택

- [ ] OTA 펌웨어 업데이트 구현 (H743 dual bank flash 활용)
- [ ] 여분 ADS131M04 채널 2개 활용처
- [ ] [[Battery SoC Estimation]] OCV 곡선 캘리브레이션 (배터리 셀 종류 확정 후)
- [ ] 부저 포함 여부 (에러 알림)

## ❓ 내부 검토 필요

- [ ] Legacy 의 **ADS1234 + INA128UA** 대안: 재논의했으나 ADS131M04 유지. 추후 필요 시 재검토 노트 링크.
- [ ] Legacy 의 **ISO1050 vs 내 추천 (없음)**: 채택. CAN-FD 필요 시 ISO1042 업그레이드 경로.
- [ ] MCU Nucleo-H743ZI 개발보드 구매 (legacy plan) — Phase A 펌웨어 선개발용.
