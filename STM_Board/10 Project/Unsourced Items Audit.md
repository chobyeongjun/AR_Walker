---
aliases: [Unsourced Audit, 출처 감사]
tags: [project, audit]
---

# Unsourced Items Audit (완료)

> 사용자 방침 (2026-04-20): "너의 생각으로 하는 것들은 모두 X. 무조건 근거 있이."
> **v3.9 에서 답변 반영 완료.** 미결 항목은 "잠정" 으로 표기.

## ✅ 해결된 항목

| # | 항목 | v3.9 결정 | 근거 |
|---|---|---|---|
| 1 | ESP32-C3-MINI-1U BLE | ✅ **포함** | 사용자: "BLE 반드시 필요" |
| 2 | Hardware SYNC GPIO | ✅ **재추가** (v3.10) | 사용자: "SYNC 추가야" (v3.9 제거 철회) |
| 3 | USB-C 디버그 | ✅ **유지** | Legacy W5 에서 USB-C 존재 전제로 리뷰 |
| 4 | JST-GH 1.25mm 통일 | ✅ **확정 (근거 있음)** | JST 공식 카탈로그 ([[Citations & Sources#F. 커넥터 근거]]) |
| 5 | 보드 사이즈 50×50mm | ⏸️ **잠정** | 사용자: "motor 정해지면 재결정" |
| 6 | ESP32 variant (-1U vs -1) | ⏸️ **잠정** | ESP32 자체 포함 확정, variant 는 케이스 설계 시 |
| 7 | SoC 쿨롱 카운팅 알고리즘 | ✅ **표준 관행** | TI SBOS882 INA228 CHARGE 레지스터 기반 |
| 8 | E-stop 이중 안전 아키텍처 | ✅ **유지** | 사용자: "E-stop 필수", SIL 표준 권장안 |
| 9 | ADS131M04 선택 | ✅ **사용자 확정** | 데이터시트 기반 비교 후 사용자 "위 권고대로" |
| 10 | H743 핀 할당 | 🚩 **대부분 CubeMX 확정 필요** | FDCAN1 외 AF 매핑 추측 |
| 11 | ISO1050 격리 CAN | ✅ **Legacy 출처** | BOARD_DESIGN_REVIEWED |
| 12 | BGA vs LQFP 분석 | ✅ **데이터시트 + 사용자 동의** | |
| 13 | 보호 회로 수치 일체 | ✅ **Legacy 전부** | EXOSUIT_PROTECTION |

## 🚩 남은 "재확인 필요"

### STM32H743VIT6 핀 할당 (CubeMX `.ioc` 필요)

| 핀 그룹 | 상태 |
|---|---|
| HSE, LSE, SWDIO, SWCLK, VCAP × 2, VDDA, VREF+, FDCAN1, USB FS | ✅ ST 고정핀 또는 legacy 확정 |
| SPI1, SPI3, I²C1, UART2/3/4, TIM1, VBUS_SENSE, ESTOP, MOTOR_ENABLE | 🚩 CubeMX 에서 충돌 검사 필수 |

**액션:** CubeMX 돌려서 `.ioc` 에 확정된 핀을 넣고 → atomic 노트 테이블 재작성.
→ [[Citations & Sources]] "CubeMX 란?" 섹션 참고.

### 4 모터 → 2 모터

Legacy 보호 수치 (1J 재생, 11,280µF 벌크 등) 는 **4 모터 가정**. 우리는 **2 모터**:
- 재생 에너지: 1J → ~0.5J (여유)
- 벌크 캡: 11,280µF → 더 적어도 OK?
- **Phase B 에서 재계산 필수**

### 24V vs 48V

모든 보호 부품 (P6KE33CA, SMAJ28CA, 벌크 캡 35V 정격, DMP2305U -30V 등) 은 24V 기준. 48V 로 가면:
- 벌크 V 정격: 35V → 63V
- TVS 클램프: 33V → 68V
- 등등 → [[Voltage-Current Scaling]] 매트릭스

### 가격 정보 (BOM)

내 기억 기반 (~2024-2025) — 발주 전 **실시간 재확인 필수**.

## 관련

- [[Citations & Sources]]
- [[Decisions Log]]
- [[Open Questions]]
