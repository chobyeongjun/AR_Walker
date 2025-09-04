#ifndef ERROR_REPORTER_H
#define ERROR_REPORTER_H

#include "UARTHandler.h"
#include "uart_commands.h"
#include "error_codes.h"
#include "ParseIni.h"
#include "UART_msg_t.h"

class ErrorReporter
{
    ErrorReporter() {};
    ~ErrorReporter() {};

public:

    static ErrorReporter *get_instance() // 싱글톤 패턴을 사용하여 ErrorReporter 객체를 생성하고 반환하는 함수
    {
        static ErrorReporter instance; // 스택에 static 객체 생성 (메모리 누수 없음)
        return &instance;
    }


    void report(ErrorCodes error_code, config_defs::joint_id joint_id) // error_code와 joint_id를 받아서 UARTHandler를 통해 다른 MCU로 에러 메시지를 전송하는 함수
    {
        UART_msg_t msg;
        msg.joint_id = static_cast<uint8_t>(joint_id);
        msg.data[(uint8_t)UART_command_enums::get_error_code::ERROR_CODE] = (float)static_cast<int>(error_code);
        UART_command_handlers::get_error_code(
            UARTHandler::get_instance(), nullptr, msg);
    }
};

#endif