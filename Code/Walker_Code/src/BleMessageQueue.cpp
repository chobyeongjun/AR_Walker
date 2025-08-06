#include "Arduino.h"
#include "BleMessageQueue.h"
#include "Logger.h"

// For mutex lock
#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
#include "mbed.h"
#include "rtos.h"
static rtos::Mutex queue_mutex; // 멀티태스킹 환경에서 큐 접근을 보호하기 위한 뮤텍스
#endif

#define BLE_QUEUE_DEBUG 0

static const int k_max_size = 10;
static BleMessage queue[k_max_size]; // BleMessage 객체를 저장할 정적 배열 (실제 큐 저장소)
static int m_size = 0;
static const BleMessage empty_message = BleMessage();

BleMessage ble_queue::pop()
{
#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
    queue_mutex.lock(); // 큐접근 잠금
#endif

    if (ble_queue::size()) // m_size가 0보다 큰지 (큐가 비어있는지 )
    {
        BleMessage msg = queue[m_size];
        // BleMessage msg = queue[m_size - 1] 마지막 요소는 m_size - 1 인덱스에
        m_size--;
        return msg;

        /* 올바른 FIFO pop을 위해
        BleMessage msg_to_return = queue[0]; // 큐의 맨 앞(가장 오래된) 메시지
        for (int i = 0; i < m_size; i++) { // 나머지 메시지들을 한 칸씩 앞으로 당김
        queue[i] = queue[i+1];
            */
    }
    else // 큐가 비어있으면
    {
        logger::println("BleMessageQueue::pop_queue->No messages in Queue!", LogLevel::Warn);
        return empty_message;
    }

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
    queue_mutex.unlock();
#endif
}

void ble_queue::push(BleMessage *msg)
{
#if BLE_QUEUE_DEBUG
    logger::println("BleMessageQueue::push_queue");
    logger::print("m_size: ");
    logger::println(m_size);
    logger::print("msg: ");
    BleMessage::print(*msg);
#endif

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
    queue_mutex.lock();
#endif

    if (m_size >= (k_max_size - 1))
    {
        logger::println("BleMessageQueue::push_queue->Queue Full!", LogLevel::Warn);
        return;
    }

    m_size++;
    queue[m_size].copy(msg);

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
    queue_mutex.unlock();
#endif

#if BLE_QUEUE_DEBUG
    logger::print("m_size: ");
    logger::println(m_size);
    logger::print("msg: ");
    BleMessage::print(queue[m_size]);
#endif
}

int ble_queue::size()
{
    return m_size;
}

int ble_queue::check_for(BleMessage msg) // : 큐 내에서 특정 메시지와 일치하는 메시지의 개수
{
    int found = 0;
    for (int i = 0; i < m_size; i++)
    {
        found += BleMessage::matching(queue[i], msg);
    }
    return found;
}

void ble_queue::clear()
{
    m_size = 0;
}