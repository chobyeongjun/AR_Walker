/**
 * @file GattDb.h
 * @author Chancelor Cuddeback
 * @brief Defines the GATT database for the BLE interface
 * @date 2023-07-18
 *
 */

#ifndef GATT_DB_H
#define GATT_DB_H

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
#include "Arduino.h"
#include "ArduinoBLE.h"

class GattDb
{
private:
    const uint8_t BUFFERS_FIXED_LENGTH = false;

public:
    const uint8_t BUFFER_SIZE = 255;
    // https://stackoverflow.com/questions/10052135/expected-identifier-before-string-constant

    // UUID (Universally Unique Identifier) : 전 세계적으로 중복되지 않도록 만들어진 고유한 ID 번호
    // Nordic Semiconductor사가 정의한 **'UART Service'**로 매우 널리 알려진 표준 UUID
    // 블루투스를 통해 시리얼 통신(UART)을 흉내 낼 때 사용되는 사실상의 표준
    // 심박수 측정(Heart Rate), 배터리 잔량(Battery Level), 그리고 코드에 나온 UART 서비스처럼 매우 흔하게 사용되는 기능들은, 전 세계 개발자들이 모두 똑같은 UUID를 사용하도록 블루투스 표준 기관에서 미리 정해두었습니다.        BLEService UARTService{"6E400001-B5A3-F393-E0A9-E50E24DCCA9E"};
    BLEService UARTService{"6E400001-B5A3-F393-E0A9-E50E24DCCA9E"};

    BLECharacteristic TXChar{"6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLERead | BLENotify | BLEBroadcast, BUFFER_SIZE, BUFFERS_FIXED_LENGTH};
    BLECharacteristic RXChar{"6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWriteWithoutResponse | BLEWrite | BLENotify, BUFFER_SIZE, BUFFERS_FIXED_LENGTH};

    // Service and Characteristics for exo info
    //  이 UUID들은 표준 UUID가 아닌, 이 프로젝트의 개발자가 직접 생성한 커스텀 UUID
    //  UUID 생성기 같은 도구를 사용해서 올바른 형식으로 새로운 UUID를 만든다면 기술적으로는 전혀 문제없을듯
    BLEService UARTServiceDeviceInfo{"e0271458-8c6a-11ed-a1eb-0242ac120002"};                                           // Serivce UUID for sending exo data
    BLECharacteristic PCBChar{"e0271459-8c6a-11ed-a1eb-0242ac120002", BLERead, BUFFER_SIZE, BUFFERS_FIXED_LENGTH};      // Characteristic for pcb
    BLECharacteristic FirmwareChar{"e0271460-8c6a-11ed-a1eb-0242ac120002", BLERead, BUFFER_SIZE, BUFFERS_FIXED_LENGTH}; // Characteristic for firmware
    BLECharacteristic DeviceChar{"e0271461-8c6a-11ed-a1eb-0242ac120002", BLERead, BUFFER_SIZE, BUFFERS_FIXED_LENGTH};   // Characteristic for device

    // Service and CHaracteristics for error reporting
    BLEService ErrorService{"33b65d42-611c-11ed-9b6a-0242ac120002"};                                                             // Service for error reporting
    BLECharacteristic ErrorChar{"33b65d43-611c-11ed-9b6a-0242ac120002", BLERead | BLENotify, BUFFER_SIZE, BUFFERS_FIXED_LENGTH}; // Characteristic for error reporting
};

#endif
#endif
