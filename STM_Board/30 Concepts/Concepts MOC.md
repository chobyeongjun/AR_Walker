---
aliases: [Concepts]
tags: [moc, concepts]
---

# Concepts MOC

재사용 가능한 엔지니어링 개념 / 설계 선택 근거.

## 전원·보호

- [[eFuse]] — 전자 퓨즈 통합 보호
- [[Motor Power Isolation]] — 모터 전원과 로직 분리 원칙
- [[Voltage-Current Scaling]] — 전압 vs 전류 트레이드오프
- [[Battery SoC Estimation]] — 잔량 추정 알고리즘

## 안전

- [[E-stop Dual Safety]] — SW EXTI + HW AND gate 이중 차단

## 통신

- [[Hardware Sync for Jetson]] — MCU-Jetson 시계 동기

## 신호 / 측정

- [[Loadcell Amplifier Design]] — 24-bit ADC 안티앨리어싱

## 부품 선정

- [[BGA vs LQFP]] — 패키지 트레이드오프

## PCB

- [[PCB Stackup 6-layer]] — 스택업·제조 사양

## 프로세스

- [[Modular Phase A-B Strategy]] — 모터 무관 / 의존 분리 전략
