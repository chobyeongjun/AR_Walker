#ifndef EXOBLE_H
#define EXOBLE_H
#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#define EXOBLE_DEBUG      //Uncomment if you want to print debug statements to the serial monitor
#define MAX_PARSER_CHARACTERS 8
#define NAME_PREAMBLE "ARLAB_"
#define MAC_ADDRESS_TOTAL_LENGTH 17
#define MAC_ADDRESS_NAME_LENGTH 6

#include <Arduino.h>
#include <ArduinoBLE.h>

#include "BleParser.h"
#include "GattDb.h"
#include "BleMessage.h"
#include "BleMessageQueue.h"

class ExoBLE
{
public:
   
    ExoBLE();
 
    bool setup();

    void advertising_onoff(bool onoff);

    bool handle_updates();

    void send_message(BleMessage &msg);

    void send_error(int error_code, int joint_id);

private:
    // BLE connection state
    int _connected = 0;

    // The Gatt database which defines the services and characteristics
    GattDb _gatt_db = GattDb();

    // The parser used to serialize and deserialize the BLE data
    BleParser _ble_parser = BleParser();
};


namespace ble_rx
{
    void on_rx_recieved(BLEDevice central, BLECharacteristic characteristic);
}

namespace connection_callbacks
{
    static bool is_connected = false;
}

#endif // defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#endif