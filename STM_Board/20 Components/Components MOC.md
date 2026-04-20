---
aliases: [Components]
tags: [moc, components]
---

# Components MOC

부품별 atomic 노트 인덱스.

## Phase A (모터 무관, 지금 freeze)

### MCU & 주변
- [[STM32H723VGT6]] — 메인 MCU
- [[ESP32-C3-MINI-1U]] — BLE/WiFi 무선

### 센서 / ADC
- [[ADS131M04]] — 24-bit 로드셀 ADC
- [[REF5025]] — 2.5V 정밀 레퍼런스
- [[AS5048A]] — SPI 자기식 인코더
- [[INA228]] — 배터리 모니터 (85V)

### 통신
- [[TCAN1462]] — CAN-FD 트랜시버
- [[TPD4S012]] — USB ESD

### 전원·보호
- [[TPS62933]] — 2.5MHz buck
- [[TLV75533]] — 3.3V LDO
- [[LTC4368-1]] — eFuse 컨트롤러
- [[IPB180N06S4]] — 60V/180A N-MOSFET

### 안전·UI
- [[74LVC1G08]] — AND gate (E-stop HW)

### 저장
- [[DM3AT microSD]] — push-push 슬롯

## Phase B (모터 결정 후)

- 모터 전압·전류 확정 후 아래 부품 선정:
  - XT60 / XT90 / Molex 커넥터
  - 벌크 캡 (V 정격)
  - TVS (SMCJ 클램프 전압)
  - Common-mode choke (전류 정격)
  - 백업 blade 퓨즈 (전류 정격)

→ [[Motor Power]] 블록 참고
