---
aliases: [Motor Power Isolation, 모터 전원 분리]
tags: [type/concept, domain/power, domain/pcb]
type: concept
---

# Motor Power Isolation

## 문제

모터 전원 (24~48V, 15A+ 연속) 이 **보드를 통과** → 로직 영역으로 노이즈·회생 전류·EMI 침투 시 ADC 정밀도·통신 신뢰성 붕괴.

## 해법: 전원·GND·레이아웃 3-레이어 분리

### 1. 전원 분리

```
24~48V in ─► [[eFuse|eFuse (LTC4368)]] ─┬─► 모터 출력 (Elmo 드라이버)
                                         │
                                         └─► ferrite bead ─► [[TPS62933]] Buck ─► 5V
                                                                                   │
                                                                         [[TLV75533]] digital 3V3
                                                                         [[TLV75533]] analog 3V3 (추가 ferrite)
```

- **Ferrite bead** (100MHz 고주파만 차단, DC 직접 통과): BLM18PG471 또는 동급
- 아날로그 3V3 은 **2단 ferrite 분리** → ADS131M04 SNR +6dB

### 2. GND 분리 (Star ground)

```
Motor GND (모터 전원 회생·스위칭 노이즈 강함)
     │
     ▼
     ●  ← 단일점 연결 (star point, 입력 커넥터 근처)
     │
     ▼
Logic GND (모든 로직 IC·MCU·ADC 는 여기로)
```

- GND 평면 2개 ([[PCB Stackup 6-layer]] L2, L5) 로 물리적 분리
- 단일점 연결 포인트는 보드 내 한 곳 (eFuse 출력 직후 권장)

### 3. PCB 레이아웃 규칙

- 모터 전원 영역을 **보드 한쪽에 몰아서** 배치 (예: 왼쪽 1/3)
- 아날로그 측정 영역 ([[Loadcell Amp]]) 은 반대쪽 배치
- 모터 폴리곤과 신호 영역 **3mm 이상 간격**
- 모터 트레이스 위 다른 신호 라우팅 금지 (인덕티브 결합)
- 2oz 외층 동박 ([[PCB Stackup 6-layer]]) + 200mil 너비 → 30A 커버
- 비아 분산 배치 (한 라인에 8개 이상)

### 4. 필터 컴포넌트

- **Common-mode choke** (Wurth 7443631400, 60A) — EMI 차단
- **Y-cap** (1nF 1kV 세라믹) — 라인-GND CM 노이즈
- **벌크 캡** 분산 (470 µF Al-poly × 4 + 22 µF X7R × 4)

## 왜 중요한가

- 모터 스위칭 → 수십 kHz 고조파 방사
- 회생 전류 → 급격한 dV/dt → 민감한 ADC 에 직격
- GND 공유 시 ground bounce → 로드셀 측정 오차

## 관련

- [[eFuse]]
- [[PCB Stackup 6-layer]]
- [[Loadcell Amplifier Design]]
- [[Motor Power]]
