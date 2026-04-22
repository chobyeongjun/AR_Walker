---
aliases: [E-stop, Emergency Stop, 긴급정지]
tags: [type/concept, domain/safety]
type: concept
---

# E-stop Dual Safety

## 문제

긴급정지는 **MCU 가 프리즈해도 동작해야** 한다. SW interrupt 만으로는 불안전.

## 해법: SW + HW 이중 차단

```
┌──────────────┐         ┌────────────────┐
│ External NC  │  J_ESTOP│ Pull-up        │
│ Mushroom Btn │─────────│ + 디바운스     │─┬─► MCU EXTI (SW 차단)
└──────────────┘         │ R=10k, C=10nF  │ │
                         └────────────────┘ │
                                            ├─► AND gate 입력 (HW 차단)
                                            │
          MCU motor_enable GPIO ────────────┴─► AND gate 입력
                                            │
                              motor_enable_final ◄ AND gate 출력
                                            │
                                            ▼
                              모터 드라이버 enable / SSR 게이트
```

### SW 경로
- MCU EXTI 인터럽트 → 즉시 disable 명령 송신 + motor_enable LOW
- 빠른 응답, 상태 로깅 가능

### HW 경로
- [[74LVC1G08]] AND 게이트가 직접 enable 신호 끊음
- MCU 가 멈춰도(펌웨어 행, 디버거 정지) 무관하게 동작
- **MCU 우회 안전**

## 부품

- 외부: 산업용 NC mushroom 버튼 (사용자 마련)
- 보드:
  - JST-GH 2pin (NC 접점 입력)
  - 10 kΩ 풀업
  - 10 nF 디바운스 캡
  - 1 kΩ 직렬 저항
  - **[[74LVC1G08]]** AND gate (SC-70-5, 1.6×1.6mm)
  - 적색 LED (E-stop 상태)
  - (옵션) 74LVC1G373 RS-latch — 누른 후 reset 버튼 눌러야 복귀

## 동작

1. 평시: NC 접점 닫힘 → ESTOP_OK = 3.3V → AND gate 통과 → motor_enable 유효
2. E-stop 누름: NC 접점 열림 → ESTOP_OK = 0V → AND gate 출력 LOW → **모터 드라이버 즉시 disable**
3. MCU EXTI 도 동시 트리거 → SW 레벨에서도 상태 기록
4. Latch 사용 시: reset 버튼 누를 때까지 유지

## 면적

~25 mm² — [[Safety UI]] 블록 안에 배치.

## 관련

- [[74LVC1G08]]
- [[Safety UI]]
- [[Motor Power Isolation]]
- [[Goals & Requirements]]
