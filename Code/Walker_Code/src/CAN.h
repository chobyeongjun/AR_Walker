/**
 * @file CAN.h
 * @author Chancelor Cuddeback
 * @brief Uses the FlexCan library to send and receive CAN messages.
 * @date 2023-07-18
 * 
 */

#ifndef CAN_H
#define CAN_H

#include "Logger.h"
#include "Arduino.h"

 //Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "FlexCAN_T4.h"
#if defined(ARDUINO_TEENSY36)
    static FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> Can0;
#elif defined(ARDUINO_TEENSY41)
    static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0; 
    // FlexCAN_T4: CAN 통신 라이브러리 클래스 이름  
    // Teensy 4.x 보드에 내장된 FlexCAN 컨트롤러를 사용하여 CAN 통신을 쉽게 할 수 있도록 기능을 제공
    // <CAN1, RX_SIZE_256, TX_SIZE_16>: FlexCAN_T4 클래스의 템플릿(template) 매개변수
    // CAN1 : Teensy 4.x 보드에 내장된 CAN 버스 컨트롤러 중 첫 번째 것을 사용하겠다는 의미
    // Can0 : FlexCAN_T4 클래스를 사용하여 생성된 CAN 통신 객체의 이름
    // 이 객체를 통해 CAN 버스를 초기화하고, 메시지를 보내거나(transmit) 받을(receive) 수 있게 됩니다. (예: Can0.begin(), Can0.write(), Can0.read() 등의 함수를 호출하여 CAN 통신을 제어
#endif

/**
 * @brief CAN class for sending and receiving CAN messages. Singleton
 * 
 */
class CAN 
{
    public:
        /**
         * @brief Get the Singleton object
         * 
         * @return CAN* 
         */
        static CAN* getInstance()
        {
            static CAN* instance = new CAN;
            return instance;
        }

        /**
         * @brief Send a CAN message
         * 
         * @param msg CAN_message_t to send
         */
        void send(CAN_message_t msg)
        {
            if(!Can0.write(msg)) 
            {
                logger::println("Error Sending" + String(msg.id), LogLevel::Error);
            }
        }

        /**
         * @brief Read a CAN message
         * 
         * @return CAN_message_t 
         */
        CAN_message_t read()
        {
            CAN_message_t msg;
            Can0.read(msg);
            return msg;
        }

    private:
        /**
         * @brief Construct a new CAN object and initialize the CAN bus. This is private 
         * because this is a singleton.
         * 
         */
        CAN()
        {   
            Can0.begin();
            Can0.setBaudRate(1000000);
        }
};

#endif
#endif