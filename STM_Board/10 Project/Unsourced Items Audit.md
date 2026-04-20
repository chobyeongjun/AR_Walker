---
aliases: [Unsourced Audit, 출처 감사]
tags: [project, audit, status/review-required]
---

# Unsourced Items Audit

> 내가 **근거 없이 내 판단으로** 넣은 항목 전수 감사. 사용자 방침: "너의 생각으로 하는 것들은 모두 X. 무조건 근거 있이."

## 분류 기준

- ✅ **Legacy 출처** — `_legacy/*.md` 에서 명시적으로 나옴
- ✅ **데이터시트 출처** — 벤더 공식 문서 값
- ✅ **사용자 확정** — 이 세션 대화에서 사용자가 명시적으로 동의
- 🚩 **내 판단** — 출처 없이 내가 추가함. **제거 또는 사용자 확정 필요**

---

## 🚩 제거 또는 확정 필요 항목

### 1. ESP32-C3-MINI-1U (무선 BLE/WiFi)

| 항목 | 상태 |
|---|---|
| Legacy | ❌ 없음 |
| 데이터시트 | ❌ 부품은 있지만 "포함하라"는 근거 아님 |
| 사용자 확정 | ❌ 명시적 요청 없음 |

**현재 노트:** `20 Components/ESP32-C3-MINI-1U.md`, `40 Schematic Blocks/Wireless.md`, BOM v3 에 포함, `Block Diagram.md` 에 포함

**🚩 판정:** 내가 추가함. 사용자 "BLE·WiFi 원해?" 물어본 적 없음.
**액션:** 삭제 or 사용자 확정.

---

### 2. Hardware SYNC GPIO (Jetson-MCU 시계 동기)

| 항목 | 상태 |
|---|---|
| Legacy | ❌ 없음 |
| 데이터시트 | ❌ 개념 자체가 설계 추가 |
| 사용자 확정 | △ 내가 설명 후 사용자 "응, SYNC는 왜?"만 물음, 명시 동의 아님 |

**현재 노트:** `30 Concepts/Hardware Sync for Jetson.md`, `40 Schematic Blocks/Comms.md` 에 핀 · JST-GH 3pin 커넥터 포함

**🚩 판정:** 내 추가. legacy 에서 Jetson 통신은 UART 만 언급, SYNC 없음.
**액션:** 삭제 or 사용자 확정.

---

### 3. USB-C 디버그 포트

| 항목 | 상태 |
|---|---|
| Legacy | △ BOARD_DESIGN_REVIEWED W5 에 "USB Type-C CC1/CC2 핀 미처리" 언급 → **있다는 전제** |
| 데이터시트 | TPD4S012 는 있음 |
| 사용자 확정 | ❌ |

**판정:** legacy W5 에서 USB-C 존재 전제로 review 함 → ✅ 사실상 legacy 암시.
**액션:** **유지 OK** (legacy 간접 출처).

---

### 4. JST-GH 1.25mm 커넥터 통일

| 항목 | 상태 |
|---|---|
| Legacy | △ `BOARD_DESIGN_REVIEWED` 에 "JST-GH 4핀 CAN 커넥터" OK 항목 명시 |
| 데이터시트 | ❌ |
| 사용자 확정 | ❌ |

**판정:** legacy 는 CAN 만 JST-GH 언급 — **모든 커넥터 통일**은 내 판단.
**액션:** 커넥터 선택을 부품별 Phase B 로 미루거나, 사용자 확정.

---

### 5. 6-layer 50×50 mm 보드 사이즈

| 항목 | 상태 |
|---|---|
| Legacy | ❌ 사이즈 명시 없음, layer 수 명시 없음 |
| 데이터시트 | ❌ |
| 사용자 확정 | △ "컴팩트", "Layer 높여서 사이즈 줄이자" — **구체 50×50 확정 아님** |

**현재 노트:** `PCB Stackup 6-layer.md`, `Size Budget.md` 전체가 50×50 기반

**🚩 판정:** 6-layer 는 사용자 동의, **50×50 구체 수치는 내 계산 기반**. 실제 레이아웃 돌려봐야 검증.
**액션:** "목표 (잠정)" 로 표기, P6 (PCB 레이아웃) 에서 확정.

---

### 6. ESP32 IPEX 외부 안테나 (`-1U`) 선택

| 항목 | 상태 |
|---|---|
| Legacy | ❌ ESP32 자체가 없음 |
| 데이터시트 | `-1` (PCB 내장) / `-1U` (IPEX) 두 variant 모두 Espressif 지원 |
| 사용자 확정 | ❌ |

**판정:** ESP32 자체가 내 판단이고, variant 선택도 내 판단.
**액션:** ESP32 삭제되면 자동 해결, 유지되면 사용자 확정 필요.

---

### 7. Battery SoC Estimation — 쿨롱 카운팅 알고리즘 세부

| 항목 | 상태 |
|---|---|
| Legacy | ❌ 구체 알고리즘 없음, 단 INA228 × 2 자체는 legacy |
| 데이터시트 | INA228 SBOS882 에 CHARGE 레지스터 존재·기능 명시 |
| 사용자 확정 | ❌ "배터리 잔량 확인해야지" 요청 있음 (general) |

**판정:** INA228 활용은 legacy · 데이터시트, **SoC 추정 OCV 곡선·재캘리브레이션 주기 등은 내 설계**.
**액션:** "펌웨어 구현 예시, 실측 캘리브레이션 필수" 로 명시. 알고리즘 자체는 표준 관행.

---

### 8. E-stop Dual Safety (SW EXTI + HW AND gate 이중)

| 항목 | 상태 |
|---|---|
| Legacy | △ HANDOFF 에 E-stop 언급 X, BOARD_DESIGN 검토 안 됨. **이중 안전 구조는 내 제안** |
| 데이터시트 | 74LVC1G08 은 일반 AND gate |
| 사용자 확정 | ✅ 사용자 "긴급정지도 만들어야 해" |

**판정:** **E-stop 존재는 사용자 확정**, 구체 이중 안전 (AND gate) 아키텍처는 내 판단.
**액션:** "사용자 확정: E-stop 필수. 아키텍처는 표준 기능안전(SIL 개념) 권장안" 으로 명시.

---

### 9. ADS131M04 최종 채택 이유

| 항목 | 상태 |
|---|---|
| Legacy | ❌ legacy 는 ADS1234+INA128UA 선정 |
| 데이터시트 | SBAS950 에 스펙 명시 |
| 사용자 확정 | ✅ "위 권고대로 바꿈" — 사용자 결정 |

**판정:** 부품 선정은 **사용자 확정**. 이유 분석은 내 기술 비교 — 데이터시트 근거.
**액션:** 유지 OK. 하지만 `RECONCILIATION.md` 에 "user decision" 임을 명시.

---

### 10. STM32H743VIT6 핀 할당 (FDCAN1 외)

| 항목 | 상태 |
|---|---|
| Legacy | ✅ FDCAN1 (PD0/1), VCAP (Pin 33, 57), VREF+ (Pin 21), VDDA (Pin 11), USB-C CC |
| 데이터시트 | DS12110 에 LQFP100 핀아웃 + AF 매핑 전체 |
| 사용자 확정 | ❌ 세부 핀 |

**🚩 문제:** 내가 제안한 SPI1/SPI3/I²C1/UART2/UART3/UART4/USB FS/TIM1/SYNC/ESTOP 핀들은 **AF 매핑 기반 내 추측** — 실제 충돌 검사 안 됨.

**판정:** legacy 확정된 핀만 신뢰. 나머지는 **CubeMX 에서 확정 필요**.
**액션:** `STM32H743VIT6.md` · `MCU Core.md` 핀 테이블에 "🚩 CubeMX 확정 필요" 표기.

---

### 11. CAN 트랜시버 ISO1050 선택

| 항목 | 상태 |
|---|---|
| Legacy | ✅ BOARD_DESIGN_REVIEWED 에 "TJA1051 → ISO1050 격리" 변경 명시 |
| 데이터시트 | SLLS983 |
| 사용자 확정 | ✅ GND bounce 설명 후 수용 |

**판정:** **Legacy 1차 출처** — 내 판단 아님. 유지 OK.

---

### 12. BGA vs LQFP 비교 분석 내용

| 항목 | 상태 |
|---|---|
| Legacy | ❌ 없음 |
| 데이터시트 | ST 데이터시트에 패키지별 사양 |
| 사용자 확정 | ✅ 사용자 질문했고 LQFP 동의 |

**판정:** 비교 **팩트** 는 데이터시트 기반, **"LQFP 권장" 결론** 은 내 판단이지만 사용자 동의.
**액션:** 유지, "사용자 동의" 명시.

---

### 13. Inrush · Brake · Regen · Protection 수치 일체

| 항목 | 상태 |
|---|---|
| Legacy | ✅ EXOSUIT_PROTECTION.md 에 모든 수치 (1J, 143V, 11,280µF, 26.5V, 4Ω NTC, 3Ω/50W 등) |
| 사용자 확정 | ✅ legacy 그대로 채택 |

**판정:** **전부 legacy 출처**. 내 판단 아님. 유지.

---

## 📊 요약 매트릭스

| 항목 | 출처 | 액션 |
|---|---|---|
| 1. ESP32-C3-MINI-1U | 🚩 내 추가 | **사용자 결정 필요** |
| 2. Hardware SYNC GPIO | 🚩 내 추가 | **사용자 결정 필요** |
| 3. USB-C 디버그 | ✅ Legacy 암시 | 유지 |
| 4. JST-GH 통일 | 🚩 부분 내 판단 | **사용자 결정 필요** |
| 5. 50×50mm 사이즈 | △ 사용자 일부 동의 + 내 구체값 | "잠정" 표기, P6 확정 |
| 6. ESP32 IPEX variant | 🚩 (ESP32 포함 결정 후) | 1번 따라 |
| 7. SoC 알고리즘 세부 | △ 데이터시트 기반 | 표준 관행 명시 |
| 8. E-stop 이중 구조 | ✅ 사용자 요청 + 아키텍처는 표준 | "표준 권장안" 명시 |
| 9. ADS131M04 | ✅ 사용자 확정 | 유지 |
| 10. H743 핀 (FDCAN 외) | 🚩 AF 매핑 추측 | **CubeMX 확정 필요 표기** |
| 11. ISO1050 | ✅ Legacy | 유지 |
| 12. BGA vs LQFP 분석 | ✅ 데이터시트 + 사용자 동의 | 유지 |
| 13. 보호 회로 수치 | ✅ Legacy 전부 | 유지 |

## 🎯 4가지 질문 (사용자 답 필요)

진행하기 전에 결정해주면 노트들 정리됨:

### Q1. ESP32-C3-MINI-1U (BLE/WiFi 무선 모듈) 포함?
- **포함** → BLE 로 폰 앱·OTA·디버그 텔레메트리 가능. 보드 면적 +219 mm²
- **제거** → 보드 단순화. 무선 필요 시 외부 모듈 UART 로 연결
- → legacy 는 없음. 사용자 선택.

### Q2. Hardware SYNC GPIO (Jetson-MCU 동기) 포함?
- **포함** → 1 kHz 펄스로 타임스탬프 정합. GPIO 2개 + JST-GH 3pin 소모
- **제거** → UART 만으로 통신. 지연 허용 가정
- → legacy 는 없음. 사용자 선택.

### Q3. 모든 커넥터 JST-GH 1.25mm 통일?
- **JST-GH 통일** → 컴팩트
- **용도별 (신호 GH / 전원 XT) 혼용** → legacy 스타일
- → legacy 는 CAN 만 JST-GH, 나머지 미명시.

### Q4. 보드 사이즈 **50×50mm** 확정? 아니면 P6 PCB 에서 실측 후 결정?
- **50×50 확정** → 부품 배치 강제, 여유 없으면 재선정
- **잠정, P6 에서** → 먼저 부품 풋프린트 → 실측 → 최적 크기

답 주면 각 항목에 대해:
- 포함 → 내 노트에서 `✅ 사용자 확정` 태그
- 제거 → 관련 atomic 노트 삭제 + MOC 업데이트

## 📝 액션 플랜 (Q1~Q4 답 후)

1. 제거 결정 항목 → 해당 atomic 노트 `status/rejected` 로 마크 (백업 유지)
2. 유지 결정 항목 → 각 노트 상단에 "✅ 사용자 확정 (2026-04-20)" 배너
3. `Citations & Sources.md` 업데이트
4. 핀 할당 테이블에 "🚩 CubeMX 확정 필요" 표기 (Q5 무관 진행)
5. BOM v4 로 반영
6. Commit + push

## 관련

- [[Citations & Sources]] — 전체 출처 정리
- [[Exosuit Board MOC]]
- [[Open Questions]]
