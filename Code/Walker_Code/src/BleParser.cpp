#include "BleParser.h"
#include "ble_commands.h"
#include "Utilities.h"
#include "BleMessageQueue.h"
#include "Logger.h"

#define BLE_PARSER_DEBUG 0 // Make 1 if you want to enable Debug prints

BleParser::BleParser()
{
    ;
}
// raw data를 해석하는 함수
BleMessage *BleParser::handle_raw_data(char *buffer, int length)
{
#if BLE_PARSER_DEBUG
    logger::println("BleParser::handle_raw_data");
    logger::print("length: ");
    logger::println(length);
    logger::print("buffer: ");
    for (int i = 0; i < length; i++)
    {
        logger::print(buffer[i]);
        logger::print(" ");
    }
    logger::print("\n");
#endif

    static BleMessage *return_msg = new BleMessage();

    // If we are not waiting for data, then we are expecting a new command
    if (!_waiting_for_data)
    {
        _handle_command(*buffer); // 버퍼의 첫 번째 바이트를 명령으로 처리

        // 명령을 처리한 후, 메시지가 바로 완성되었는지 확인
        // If the message is complete, then we can return it
        if (_working_message.is_complete)
        {
#if BLE_PARSER_DEBUG
            logger::print("BleParser::handle_raw_data->message is complete\n");
#endif
            // copy는 Blemessage의 주소가 들어감
            return_msg->copy(&_working_message); // 완성된 메시지를 return_msg에 복사
            _working_message.clear();            // 작업 중인 메시지 초기화
            _waiting_for_data = false;           // 더 이상 데이터를 기다리지 않음
        }
        else // 메시지가 아직 완성되지 않았고, 추가 데이터가 필요한 경우
        {
#if BLE_PARSER_DEBUG
            logger::print("BleParser::handle_raw_data->message is not complete\n");
#endif
            //_waiting_for_data = true;
        }
    } // 2. 이전에 명령을 받았고, 이제 그 명령에 대한 데이터를 기다리는 중인 경우
    else
    {
        // Add the data packet to the working message
        //  수신된 데이터를 내부 버퍼에 추가
        memcpy(&_buffer[_bytes_collected], buffer, length);
        _bytes_collected += length; // 수집된 바이트 수 업데이트

        // If we have collected all the data that we were expecting, then we can return the message
        if (_bytes_collected == _working_message.expecting * 8) // 데이터 크기가 'expecting * 8' 바이트인 것으로 보임 (float/double 크기)
        {
            return_msg->copy(&_working_message); // 현재 작업 중인 메시지(명령 정보 포함)를 반환 메시지에 복사
            for (int i = 0; i < (_working_message.expecting * 8); i += 8)
            {
                double f_tmp = 0;
                memcpy(&f_tmp, &_buffer[i], 8);         // 8바이트를 double로 변환
                return_msg->data[i / 8] = (float)f_tmp; // double을 float으로 변환하여 메시지의 data 배열에 저장
            }
            return_msg->is_complete = true;
            _working_message.clear();
            _waiting_for_data = false;
            _bytes_collected = 0;
        }
        // If we have collected more data than we were expecting, then we reset
        else if (_bytes_collected >= _working_message.expecting * 8)
        {
            _working_message.clear();
            _waiting_for_data = false;
            _bytes_collected = 0;
        }
    }

#if BLE_PARSER_DEBUG
    logger::print("return_msg: ");
    BleMessage::print(*return_msg);
#endif

    return return_msg;
}

int BleParser::package_raw_data(byte *buffer, BleMessage &msg)
{
#if BLE_PARSER_DEBUG
    logger::print("BleParser::package_raw_data");
    BleMessage::print(msg);
#endif

    // Size must be declared at initialization because of itoa()
    char cBuffer[_maxChars] = {0};
    int buffer_index = 0;
    buffer[buffer_index++] = _start_char;
    buffer[buffer_index++] = msg.command;
    int expecting_char_length = utils::get_char_length(msg.expecting);
    itoa(msg.expecting, &cBuffer[0], 10);
    memcpy(&buffer[buffer_index], &cBuffer[0], expecting_char_length);
    buffer_index += expecting_char_length;
    buffer[buffer_index++] = _start_data;
    for (int i = 0; i < msg.expecting; i++)
    {
        double data_to_send = (double)msg.data[i];

        // Send as Int to reduce bytes being sent
        int modData = int(data_to_send * 100);
        int cLength = utils::get_char_length(modData);

        if (cLength > _maxChars)
        {
            logger::print("BleParser::package_raw_data: cLength > _maxChars\n", LogLevel::Error);
            cLength = 1;
            modData = 0;
        }

        // Populates cBuffer with a base 10 number
        itoa(modData, &cBuffer[0], 10);

        // Writes cLength indices of cBuffer into buffer
        memcpy(&buffer[buffer_index], &cBuffer[0], cLength);
        buffer_index += cLength;
        buffer[buffer_index++] = _delimiter;
    }

#if BLE_PARSER_DEBUG
    logger::print("BleParser::package_raw_data: buffer: ");
    for (int i = 0; i < buffer_index; i++)
    {
        logger::print(buffer[i]);
        logger::print(" ");
    }
    logger::print("\n");
#endif

    return buffer_index;
}

/*
 * Private Functions
 */

void BleParser::_handle_command(char command) // 명령 처리
{
#if BLE_PARSER_DEBUG
    logger::print("BleParser::_handle_command: ");
    logger::print(command);
    logger::print("\n");
#endif

    int length = -1;
    // // `ble::commands` 배열을 순회하며 입력된 `command`와 일치하는 명령 찾기
    // Get the ammount of characters to wait for
    for (unsigned int i = 0; i < sizeof(ble::commands) / sizeof(ble::commands[0]); i++)
    {
        if (command == ble::commands[i].command)
        {
            length = ble::commands[i].length;
            break;
        }
    }
    if (length == -1) // 명령 목록에서 해당 명령을 못 찾는다면
    {
        // Didnt find command in list
        _working_message.clear(); // 작업 중인 메세지 초기화
        logger::print("BleParser::_handle_command: Command is not in list: ", LogLevel::Error);
        logger::println(command, LogLevel::Error);
    }
    else // 명령을 찾는다면
    {
        _waiting_for_data = (length != 0);                 // 데이터 길이가 0이 아니면 데이터를 기다림
        _working_message.command = command;                // 작업 중인 메시지에 명령 코드 설정
        _working_message.expecting = length;               // 작업 중인 메시지에 예상 데이터 길이 설정
        _working_message.is_complete = !_waiting_for_data; // 데이터를 기다리지 않으면 메시지 완료로 간주
    }

#if BLE_PARSER_DEBUG
    logger::print("BleParser::_handle_command: ");
    BleMessage::print(_working_message);
#endif
}