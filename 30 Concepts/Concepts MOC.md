---
aliases: [Concepts]
tags: [moc, concepts]
---

# Concepts MOC

재사용 가능한 엔지니어링 개념 / 설계 선택 근거.

## 전원·보호

- [[eFuse]] — 전자 퓨즈 통합 보호 (내 원안, LTC4368)
- [[Motor Power Isolation]] — 모터 전원과 로직 분리 PCB 규칙
- [[Voltage-Current Scaling]] — 전압 vs 전류 트레이드오프
- [[Battery SoC Estimation]] — 잔량 추정 알고리즘
- **[[Regen Energy Protection]]** ⭐ 재생 에너지 143V 폭주 방지 (legacy 핵심)
- **[[Brake Resistor Circuit]]** — 26.5V 임계 자동 소산
- **[[Inrush Current Limiting]]** — NTC + 릴레이 bypass

## 안전

- [[E-stop Dual Safety]] — SW EXTI + HW AND gate 이중 차단
- **[[GND Bounce Protection]]** ⭐ 2.5V bounce → CAN 파손 근본 해결

## 통신

- [[Hardware Sync for Jetson]] — MCU-Jetson 시계 동기
- **[[Isolated CAN]]** ⭐ 5000 Vrms 격리로 bounce 차단

## 신호 / 측정

- [[Loadcell Amplifier Design]] — 24-bit ADC 안티앨리어싱, ADS131M04 근거

## 부품 선정

- [[BGA vs LQFP]] — 패키지 트레이드오프

## PCB

- [[PCB Stackup 6-layer]] — 스택업·제조 사양

## 프로세스

- [[Modular Phase A-B Strategy]] — 모터 무관 / 의존 분리 전략

## v3.5 추가 배경

[[RECONCILIATION]] 문서 참고. legacy 4-agent review 를 통해 발견된 보호 아키텍처 (재생에너지, GND bounce, 격리 CAN, 브레이크 저항, 인러시) 는 **이전 보드 파손의 근본 원인을 해결** 하는 것들. 설계 반영 필수.
