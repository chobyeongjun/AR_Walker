---
aliases: [Open Questions]
tags: [project, status/open]
---

# Open Questions

현재 미결 항목. 결정 시 [[Decisions Log]] 에 이동.

## 🔴 긴급 (Phase A 시작 전)

- [x] ~~MCU 패키지~~ → [[STM32H723VGT6]] 확정 (사용자 위임)
- [x] ~~Ethernet 포함?~~ → 완전 제거
- [x] ~~Jetson 통신 방식~~ → UART + HW SYNC
- [x] ~~USB-C 포함?~~ → 포함
- [x] ~~E-stop 포함?~~ → 포함, 이중 안전
- [x] ~~배터리 모니터?~~ → 포함

## 🟡 중요 (Phase B 시작 전)

- [ ] **모터 모델 확정** (Elmo Solo Whistle? Twitter?)
- [ ] **모터 정격 전압** (12V / 24V / 36V / 48V) — [[Voltage-Current Scaling]] 참조
- [ ] **모터 max 연속 전류 정확한 수치** (15A+ 라고 했는데 정확히?)
- [ ] **"15A" 가 전류 기준인지 전력 기준인지** (전압 낮을수록 전류 ↑)
- [ ] **회생 전류 추정값** (가속·정지 시)
- [ ] **배터리 셀 구성** (Li-ion 3S/4S/6S/…) — [[Battery SoC Estimation]] 캘리브레이션

## 🟢 낮음 (PCB 단계에서 재검토)

- [ ] 6L (50×50) vs 8L (45×45)
- [ ] 단일 보드 vs 로직+파워 2-PCB 분리
- [ ] 입력 XT60 (30A) vs XT90 (45A 마진)
- [ ] 모터 출력 XT30 vs XT60
- [ ] 8L 모터 전원 dual plane 필요 여부

## 🔵 장기 / 선택

- [ ] OTA 펌웨어 업데이트 (dual-bank flash 없어서 wireless OTA 설계 필요 여부)
- [ ] 여분 ADS131M04 채널 2개 활용처
- [ ] 부저 포함 여부 (에러 알림)
- [ ] CAN 버스 격리 (갈바닉 isolation) 필요 여부
