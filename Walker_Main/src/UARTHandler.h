
#ifndef UARTHandler_h
#define UARTHandler_h

#include "Board.h"
#include "ParseIni.h"
#include "ExoData.h"
#include "Utilities.h"
#include "UART_msg_t.h"

#include "Arduino.h"
#include <stdint.h>

#define MAX_NUM_SIDES 2             //Seems unlikely there would be any more 시스템이 다룰 수 있는 외골격 최대 부위 2와 부위당 최대 관절 수를 정의
#define MAX_NUM_JOINTS_PER_SIDE 2   //Current PCB can only do 2 motors per side, if you have made a new PCB, update.
#define MAX_RAW_BUFFER_SIZE 256 
#define MAX_DATA_SIZE 32
#define UART_DATA_TYPE short int //If type is changes you will need to comment/uncomment lines in pack_float and unpack_float
#define FIXED_POINT_FACTOR 100      // 소수점 데이터를 정수로 변환하여 보낼 때 사용할 변환 계수입니다. 예를 들어, 12.34라는 실수를 보낼 때 1234라는 정수로 바꿔 보낸 뒤, 받는 쪽에서 다시 100으로 나누어 원래 값으로 복원합니다.
#define UART_BAUD 256000           // UART 통신 속도를 256,000 bps로 설정

#define MAX_RX_LEN 64       //Bytes
#define RX_TIMEOUT_US 1000  //Microseconds

/* SLIP special character codes */
//  SLIP(Serial Line Internet Protocol)**이라는 통신 규약에서 사용하는 특수 문자 코드
#define END             0300    /* Indicates end of packet */ // END: 데이터 패킷(묶음)의 끝을 알리는 문자입니다.
#define ESC             0333    /* Indicates byte stuffing */ // ESC: 데이터 안에 END 문자와 같은 값이 있을 경우, 이를 데이터임을 알리기 위해 사용하는 이스케이프 문자
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define MY_SERIAL Serial8   // MY_SERIAL을 하드웨어 시리얼 포트인 Serial8로 정의
#elif defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
#define MY_SERIAL Serial1  // MY_SERIAL을 하드웨어 시리얼 포트인 Serial1로 정의
#else 
#error No Serial Object Found
#endif

class UARTHandler
{
    public:
       
        static UARTHandler* get_instance();


         //두 가지 버전(오버로딩)으로 정의되어 있습니다.
        void UART_msg(uint8_t msg_id, uint8_t len, uint8_t joint_id, float *buffer); // 메시지 ID, 데이터 길이, 관절 ID, 실제 데이터(float 배열)를 각각 전달하여 메시지
        void UART_msg(UART_msg_t msg);      //UART_msg_t라는 구조체에 모든 정보를 담아 한 번에 전달하여 메시지를 보냅

        UART_msg_t poll(float timeout_us = RX_TIMEOUT_US);    // poll 메서드는 UARTHandler 클래스의 인스턴스가 수신한 데이터를 확인하고, 
                                                              // 데이터가 있다면 해당 메시지를 읽어오는 역할을 합니다. 
                                                              // timeout_us 매개변수는 데이터 수신 대기 시간(마이크로초 단위)을 지정합니다. 위에서 1000ms로 설정

        uint8_t check_for_data();

    private:
        
        UARTHandler();

        void _pack(uint8_t msg_id, uint8_t len, uint8_t joint_id, float *data, uint8_t *data_to_pack);

        UART_msg_t _unpack(uint8_t* data, uint8_t len);

        uint8_t _get_packed_length(uint8_t msg_id, uint8_t len, uint8_t joint_id, float *data);

        void _send_packet(uint8_t* p, uint8_t len);

        int _recv_packet(uint8_t *p, uint8_t len = MAX_RX_LEN);

        void _send_char(uint8_t val);

        uint8_t _recv_char(void);

        uint8_t _time_left(uint8_t should_latch = 0);

        void _reset_partial_packet();

        /* Data */
        //circular_buffer<uint8_t, 64> _rx_raw;

        float _timeout_us = RX_TIMEOUT_US;
        
        uint8_t _partial_packet[MAX_RX_LEN];
        uint8_t _partial_packet_len = 0;
        uint8_t _msg_buffer[MAX_RX_LEN];
        uint8_t _msg_buffer_len = 0;

};

#endif