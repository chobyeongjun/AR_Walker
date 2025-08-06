#include "BleMessage.h"
#include "Logger.h"

BleMessage::BleMessage()
{
    this->clear(); // 객체가 생성되자마자 clear() 함수로 초기화
}

void BleMessage::clear()
{
    command = 0;
    is_complete = false;
    _size = 0;
    expecting = 0;
}

void BleMessage::copy(BleMessage *n) // 함수로 주소가 들어감
{
    command = n->command;
    is_complete = n->is_complete;
    _size = n->_size;
    expecting = n->expecting;
    for (int i = 0; i < expecting; i++)
    {
        data[i] = (n->data[i]);
    }
}

void BleMessage::print(BleMessage msg) // 주로 디버깅 목적
{
    logger::print(msg.command);
    logger::print("\t");
    logger::print(msg.is_complete);
    logger::print("\t");
    logger::print(msg.expecting);
    logger::print("\n");
    if (msg.expecting <= 0)
    {
        return;
    }
    for (int i = 0; i < msg.expecting; i++)
    {
        logger::print(msg.data[i]);
        if (i == (msg.expecting - 1))
        {
            continue;
        }
        logger::print(", ");
    }
    logger::print("\n");
}

int BleMessage::matching(BleMessage msg1, BleMessage msg2) // 두 메세지가 일치하는 지 비교
{
    int doesnt_match = 0;
    doesnt_match += msg1.command != msg2.command;
    doesnt_match += msg1.is_complete != msg2.is_complete;
    doesnt_match += msg1.expecting != msg2.expecting;
    doesnt_match += msg1._size != msg2._size;
    return (doesnt_match == 0) ? (1) : (0);
}

// data 배열의 내용은 비교하지 않는다는 점에 유의해야 합니다.
//  이는 주로 메시지의 헤더 정보나 기본 속성만으로 메시지 타입을 비교할 때 사용