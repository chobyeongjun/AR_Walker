---
aliases: [Battery SoC, SoC Estimation, 배터리 잔량]
tags: [type/concept, domain/power, domain/firmware]
type: concept
---

# Battery SoC Estimation

## 문제

배터리 잔량 (State of Charge, SoC) 을 보여줘야 함.
그런데:
- **전압만으로 SoC 추정** → 부하에 따라 전압 요동 → 부정확
- **쿨롱 카운팅만** → 시작 SoC 모르면 절대값 없음
- **풀 BMS (셀 밸런싱·충전 제어)** → 배터리 팩 자체가 담당, 우리 역할 아님

## 해법

### OCV + 쿨롱 카운팅 하이브리드 알고리즘

```
1. 부팅 시 (무부하 가정):
   OCV = INA228.BUS_VOLTAGE
   SoC_init = lookup OCV → SoC (Li-ion 곡선)

2. 운용 중:
   ΔQ = INA228.CHARGE 레지스터 변화 (쿨롱 카운팅)
   SoC(t) = SoC_init + ΔQ / 배터리 용량
   (방전 = 음수, 충전 = 양수)

3. 주기적 재캘리브레이션:
   무부하 구간 감지 (|I| < 100 mA 수초 유지)
   → OCV 재측정 → SoC 보정

4. 저전압 임계값:
   SoC < 20%: RGB LED 황색 + ESP32 알림
   SoC < 5% 또는 V < UVLO: motor enable 자동 해제
```

### Li-ion 셀당 OCV-SoC 곡선 (대략)

| SoC | 셀 전압 | 4S 팩 | 6S 팩 |
|---:|---:|---:|---:|
| 100% | 4.20 V | 16.8 V | 25.2 V |
| 80% | 4.02 V | 16.1 V | 24.1 V |
| 50% | 3.75 V | 15.0 V | 22.5 V |
| 20% | 3.55 V | 14.2 V | 21.3 V |
| 0% | 3.20 V | 12.8 V | 19.2 V |

(정확한 곡선은 셀 브랜드·온도 의존 — 실측 캘리브레이션 필요)

## 우리 보드에서의 적용

### 하드웨어
- [[INA228]] — 20-bit 전압/전류 + 내장 CHARGE 레지스터
- 1 mΩ 2W shunt — 입력 +V 라인 직렬
- 배터리 NTC 입력 (JST-GH 2pin) — MCU ADC 로 온도 읽기
- [[Battery Monitor]] 블록

### 펌웨어
- RTOS 태스크 또는 타이머에서 100ms 주기 샘플링
- 이동 평균 필터 (저역통과)
- EEPROM/플래시에 SoC 저장 (재부팅 후 유지)

## 표시 채널

| 채널 | 표시 방법 |
|---|---|
| 보드 RGB LED | 색상 (녹 > 50% / 황 20~50% / 적 < 20%) + 깜빡임 |
| ESP32 BLE → 폰 앱 | 실시간 % + 예상 사용 시간 |
| SD 카드 로깅 | 시계열 SoC, 전류, 전력 |
| Jetson UART 텔레메트리 | 실시간 % (HUD) |
| 자동 안전 | 저전압 → motor disable |

## 경계

**우리가 하지 않는 것:**
- 셀 밸런싱 (셀별 전압 관리) — 배터리 팩 BMS 담당
- 충전 제어 — 외부 충전기 담당
- 과충전 차단 — 배터리 BMS 담당

**우리 역할:** 외부에서 본 SoC 추정 + 저전압 차단만.

## 관련

- [[INA228]]
- [[Battery Monitor]]
- [[Motor Power Isolation]]
- [[Open Questions]] — 배터리 셀 구성 (3S/4S/6S/…) 캘리브레이션 필요
