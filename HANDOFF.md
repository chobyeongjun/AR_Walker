# Exosuit 보드 프로젝트 인수인계

> 날짜: 2026-04-14
> 프로젝트: 케이블 드리븐 착용형 외골격 (H-Walker 후속)

---

## 프로젝트 목표

AR_Walker(Teensy 기반) → STM32 커스텀 보드로 전환하여
케이블 드리븐 외골격의 모터 제어 + 센서 + 통신을 통합하는 새 보드 설계/제작

---

## 현재 진행 상태

### 확정된 것

| 항목 | 결정 | 비고 |
|------|------|------|
| 메인 MCU | STM32H743VIT6 (LQFP100) | 480MHz, FDCAN×2, SPI/I2C/UART 다수 |
| IMU | EBIMU EBMotion V5 (2.4GHz RF) | 1센서 1000Hz, Quaternion/Euler |
| 카메라 | Stereolabs ZED X Mini | GMSL2, 120FPS, IP67, 150g |
| 컴퓨팅 | Jetson Orin NX 16GB | 157 TOPS, ZED Link PCIe 필요 |
| 로드셀 ADC | INA128UA + ADS1234 | INA333→INA128 교체 (CMR 문제) |
| 배터리 모니터 | INA228 × 2 (85V max) | 직렬 배터리 개별 전압 측정 |
| 제어 방식 | 어드미턴스 (입각기) + 위치 PID (유각기) | 500Hz 제어 루프 |
| 모터 명령 방식 | 서보(전류) 모드 + SET_POS | MIT 모드 사용 안 함 (케이블 드리븐 특성) |
| CAN 통신 | CANopen CiA 402 | STM32 → Elmo 드라이버 |
| RTOS | FreeRTOS | STM32CubeMX 자동 생성 |

### 미결정 (다음 세션에서 결정 필요)

| 항목 | 옵션 | 결정 조건 |
|------|------|---------|
| **모터** | T-Motor U8 Lite KV85 / CubeMars RI60 KV120 / Maxon EC-i 40 | 필요 토크/전류 계산 후 |
| **모터 드라이버** | Elmo Gold Twitter / Solo Twitter / Whistle | 모터 전류 확정 후 |
| **배터리** | 24V 단일 / 24V×2 직렬(48V) | 모터 확정 후 |

---

## 핵심 기술 결정 사항 & 이유

### 1. MIT 모드 안 쓰는 이유
케이블 드리븐 → 풀리 반경 r(θ)가 관절각도에 따라 변함 → T_ff(모터 토크)를 정확히 계산하기 어려움.
서보(전류) 모드로 전류 명령 → 어드미턴스 컨트롤러가 로드셀 피드백으로 보정 → 기구학 불확실성을 제어기가 흡수.

### 2. 속도 > 토크인 이유
케이블이 "외력"으로 작용 → 사람이 주 동력, 외골격은 보조.
빠르게 따라가지 못하면 케이블 슬랙 발생 → 보조 끊김.
높은 전압 = 높은 속도 → 48V(24V×2 직렬) 검토 중.

### 3. 보드 파손 원인 (기존 시스템)
모터 4개 동시 릴리즈 → 재생 에너지 1J → 버스 캐패시터 부족 시 전압 143V 폭주.
GND bounce 2.5V → CAN 트랜시버 TXD/RXD 핀 절대최대정격(±0.3V) 초과 → 파손.

### 4. Elmo 드라이버 채택 이유
모터 전류 제어/PWM/보호 = Elmo 담당 → 커스텀 PCB 대폭 단순화.
재생 에너지 처리도 Elmo 내부 shunt (단, 용량 확인 필요).

---

## 설계 문서 위치

### stm_board 폴더
| 파일 | 내용 |
|------|------|
| `BOARD_PLAN.md` | 초기 5-Phase 마스터 플랜 |
| `BOARD_DESIGN.md` | 상세 회로 설계 (초기 버전) |
| `BOARD_DESIGN_REVIEWED.md` | **4개 에이전트 검토 후 수정본 — CRITICAL 7개 + WARNING 15개** |
| `EXOSUIT_PROTECTION.md` | **보호 회로 최종 설계 — 재생에너지/GND bounce/CAN 보호** |
| `HANDOFF.md` | 이 파일 |

### Obsidian 볼트 (`~/0xhenry.dev/vault/`)
| 파일 | 내용 |
|------|------|
| `10_Wiki/Topics/ebimu-imu.md` | EBIMU V5/V6 스펙 |
| `10_Wiki/Topics/elmo-gold-twitter.md` | Elmo Twitter 스펙 |
| `10_Wiki/Topics/elmo-driver-comparison.md` | **Elmo 전 라인업 비교표** |
| `10_Wiki/Topics/zed-x-mini.md` | ZED X Mini 스펙 |
| `10_Wiki/Topics/jetson-orin-nx.md` | Jetson 라인업 비교 + 추천 |
| `10_Wiki/Topics/motor-selection.md` | **모터 후보 3종 비교 + 인코더 호환** |
| `10_Wiki/Projects/exosuit-hardware-overview.md` | **하드웨어 총괄 아키텍처** |

---

## CRITICAL 이슈 (반드시 반영)

| # | 문제 | 수정 | 상태 |
|---|------|------|------|
| C1 | AP62200WU VIN 최대 24V < 배터리 25.2V | TPS54560B (60V)로 교체 | 설계 반영 필요 |
| C2 | SMBJ26A 클램프 42.1V > IC 절대최대정격 | TVS 재설계 | 설계 반영 필요 |
| C3 | P-MOS VGS = -22.9V > 절대최대정격 ±20V | 12V 제너(BZT52C12) 추가 | 설계 반영 필요 |
| C4 | VCAP_2 핀(Pin 57) 누락 | 2.2μF 독립 배치 | 설계 반영 필요 |
| C5 | VREF+ 핀(Pin 21) 미처리 | VDDA 직결 + 디커플링 | 설계 반영 필요 |
| C6 | ADS1234 AVDD 최소 4.75V → 3.3V 계획 | **5V 공급 필수** | 설계 반영 필요 |
| C7 | INA333 VCM 초과 (5V 여기 + 3.3V 전원) | **INA128UA로 교체** | 설계 반영 필요 |

---

## 다음 세션에서 할 일 (우선순위)

### 1순위: 필요 연속 전류 계산 ← 모든 결정의 출발점

```
필요 정보:
  - 목표 보조 토크 τ_assist (Nm) ← 보행 데이터에서 결정
  - 외골격 모멘트 암 d_moment (m) ← 프레임 설계
  - 풀리 반경 r_pulley (m) ← 기구 설계

계산:
  F_cable    = τ_assist / d_moment
  τ_motor    = F_cable × r_pulley
  I_required = τ_motor / Kt
```

**사용자 조건**: 사람의 보행 중 스윙(유각기) 시 외부에서 보조력 인가
→ 보행 데이터(정상인 70kg 기준)에서 스윙 위상 관절 토크 프로파일 확인 필요
→ 보조 비율 (10~30%) 결정 필요

### 2순위: 모터 확정
전류 계산 결과 → Kt/무게/속도 기준으로 후보 3종 중 선택

### 3순위: Elmo 드라이버 확정
모터 연속 전류에 따라:
- 20A 이하: Whistle도 가능
- 20A 초과: Twitter 또는 Solo Twitter

### 4순위: 배터리 확정
모터 전압 범위 + 속도 목표에 따라 24V 단일 or 48V 직렬

### 5순위: KiCad 스키매틱 시작
- Nucleo-H743ZI로 펌웨어 병렬 개발 가능
- CANopenNode STM32H7 포팅부터

---

## 참고 오픈소스 / TI 레퍼런스

| 소스 | 용도 |
|------|------|
| mjbots/moteus (GitHub) | STM32 + CAN 보호 회로 설계 |
| CANopenNode (GitHub) | STM32 CANopen 스택 |
| TI WEBENCH | 벅 컨버터 자동 설계 |
| TI TIDA-01255 | 48V CAN 보호 레퍼런스 |
| AR_Walker (GitHub: chobyeongjun/AR_Walker) | 기존 시스템 코드 참고 |

---

## skiro 미해결 이슈 (learnings)

```
[?] AP62200WU VIN 정격 초과 → TPS54560B 교체 필요
[?] INA333 CMR 문제 → INA128UA 교체 필요
[?] ADS1234 AVDD 5V 필요 → 전원 설계 수정 필요
```

---

## 한 줄 요약

**모터 선택이 막혀있고, 모터를 고르려면 "스윙 보조 시 필요 전류"를 먼저 계산해야 합니다. 보행 데이터 기반으로 보조 토크 → 케이블 장력 → 모터 토크 → 전류 역산이 다음 단계입니다.**
