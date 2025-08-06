#include "RealTimeI2C.h"

#include "Config.h"
#include "Utilities.h"
#include "Logger.h"
#include <Wire.h>

// #define RT_I2C_DEBUG 1

#define FIXED_POINT_FACTOR 100 // 고정소수점  부동 소수점(float)을 고정 소수점(short int)으로 변환할 때 곱하는 값 (정밀도 결정)

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define HOST 1
#if BOARD_VERSION == AK_Board_V0_3
#define MY_WIRE Wire1
#else
#define MY_WIRE Wire
#endif
#else if defined(ARDUINO_ARDUINO_NANO33BLE)
#define HOST 0
#define MY_WIRE Wire
#endif

#define RT_I2C_ADDR 0x02 // I2C 슬레이브 장치의 주소 (16진수
#define RT_I2C_REG 0x02  // I2C 레지스터 주소 (데이터 유형 식별용으로 사용될 수 있음)

static volatile bool new_bytes = false;                                                  // I2C 수신 인터럽트에서 새로운 데이터가 도착했음을 알리는 플래그
static const int byte_buffer_len = rt_data::len * sizeof(float) / sizeof(short int) + 2; // 실제 수신된 I2C 바이트 데이터를 임시 저장할 버퍼
static uint8_t *const byte_buffer = new uint8_t(byte_buffer_len);                        // byte_buffer_len 크기의 uint8_t 배열을 힙에 동적 할당
static float *float_values = new float(rt_data::len);

static uint8_t _packed_len(uint8_t len) // float 데이터의 개수(len)를 바탕으로 I2C로 전송될 패킷의 총 바이트 길이를 계산
{
    uint8_t packed_len = 0;
    packed_len += (float)len * (sizeof(float) / sizeof(short int));
    packed_len += 2; // Preamble
    return packed_len;
}

static void _pack(uint8_t msg_id, uint8_t len, float *data, uint8_t *data_to_pack) // float 배열 데이터를 I2C로 전송하기 위한 **바이트 배열로 변환(포장)**
{
    // Pack metadata
    data_to_pack[0] = msg_id;
    data_to_pack[1] = len + 2; // 패킷의 첫 2바이트는 메타데이터(메시지 ID, 길이)로 채웁니다.

    // Convert float array to short int array
    uint8_t _num_bytes = sizeof(float) / sizeof(short int);
    uint8_t buf[_num_bytes];
    for (int i = 0; i < len; i++)
    {
        utils::float_to_short_fixed_point_bytes(data[i], buf, FIXED_POINT_FACTOR);
        uint8_t _offset = (2) + _num_bytes * i;
        memcpy((data_to_pack + _offset), buf, _num_bytes);
    }
}

void real_time_i2c::msg(float *data, int len) //  (마스터 전송 함수) I2C 마스터가 슬레이브 장치로 float 데이터 배열을 전송
{
    const uint8_t packed_len = _packed_len(len);
    uint8_t bytes[packed_len];
    _pack((uint8_t)RT_I2C_REG, len, data, bytes);

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    MY_WIRE.beginTransmission(RT_I2C_ADDR);
    MY_WIRE.send(bytes, packed_len);
    MY_WIRE.endTransmission();
#endif
}

#if defined(ARDUINO_ARDUINO_NANO33BLE)
// Warning: This is an interrupt and needs to be kept as short as possible. (NO SERIAL PRINTS)
// Also, any variables that are stored outside of its scope must be marked volatile and need to have their concurrency managed
static void on_receive(int byte_len)
{
    if (byte_len != byte_buffer_len)
    {
        return;
    }
    for (int i = 0; i < byte_len; i++)
    {
        byte_buffer[i] = MY_WIRE.read();
    }
    new_bytes = true;
}
#endif

void real_time_i2c::init()
{
#if HOST // HOST가 1인 경우 (Teensy 3.6/4.1)
    MY_WIRE.begin();
#else // HOST가 0인 경우 (Nano 33 BLE)
    MY_WIRE.begin(RT_I2C_ADDR);
    MY_WIRE.onReceive(on_receive);
#endif
}

bool real_time_i2c::poll(float *pack_array) // 슬레이브 수신 데이터 처리
// on_receive 인터럽트에서 수신된 바이트 데이터를 float 형태로 역변환(언팩)하고, 이 데이터를 상위 호출자(ComsMCU::update_gui())에게 전달
{
#if RT_I2C_DEBUG
    logger::println("real_time_i2c::poll()->Start");
#endif

    if (!new_bytes)
    {
#if RT_I2C_DEBUG
        logger::println("real_time_i2c::poll()->End (no new bytes)");
#endif
        return false;
    }

    noInterrupts();                             // noInterrupts()로 인터럽트를 비활성화
    const uint8_t len = byte_buffer[1];         // byte_buffer의 데이터가 중간에 변경되지 않도록 보호
    uint8_t buff[byte_buffer_len];              // byte_buffer_len 크기의 임시 버퍼를 생성
    memcpy(buff, byte_buffer, byte_buffer_len); // byte_buffer의 내용을 buff로 복사
    new_bytes = false;
    interrupts(); // interrupts()를 다시 활성화

#if RT_I2C_DEBUG
    logger::print("real_time_i2c::poll()->Done copying bytes, len: ");
    logger::print(len);
    logger::println();
#endif

    for (int i = 0; i < (len); i++) // buff에 복사된 바이트 데이터를 utils::short_fixed_point_bytes_to_float 함수를 사용하여 하나씩 float 값으로 역변환
    {
        uint8_t data_offset = (2) + (i * 2); // Preamble plus i * sizeof(float)/sizeof(short int)
        float tmp = 0;
        utils::short_fixed_point_bytes_to_float((uint8_t *)(buff + data_offset), &tmp, FIXED_POINT_FACTOR); // FIXED_POINT_FACTOR를 사용하여 원래의 소수점 정밀도로 복원
        pack_array[i] = tmp;

#if RT_I2C_DEBUG
        logger::print("real_time_i2c::poll()->i: ");
        logger::print(i);
        logger::print(" data_offset: ");
        logger::print(data_offset);
        logger::print(" buff[data_offset]: ");
        logger::print(buff[data_offset]);
        logger::print(" buff[data_offset+1]: ");
        logger::print(buff[data_offset + 1]);
        logger::print(" tmp: ");
        logger::print(tmp);
        logger::print("\n");
#endif
    }

#if RT_I2C_DEBUG
    logger::println("real_time_i2c::poll()->End");
#endif
    return true;
}