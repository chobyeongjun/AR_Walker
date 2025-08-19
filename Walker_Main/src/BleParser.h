#ifndef BLEPARSER_H
#define BLEPARSER_H

#include "Arduino.h"
#include "ble_commands.h"
#include "BleMessage.h"
#include "GattDb.h"

class BleParser
{
public:

    BleParser();

    BleMessage *handle_raw_data(char *buffer, int length);

    int package_raw_data(byte *buffer, BleMessage &msg);

private:
    const char _start_char = 'S';
    const char _start_data = 'c';
    const char _delimiter = 'n';
    const int _maxChars = 12;
    
    //Empty message
    const BleMessage _empty_message = BleMessage();

    //The current working message, used by handle_raw_data
    BleMessage _working_message = BleMessage();
    bool _waiting_for_data = false;
    int _bytes_collected = 0;
    byte _buffer[64] = {0};

    void _handle_command(char command);
};

#endif