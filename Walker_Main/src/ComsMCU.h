#ifndef COMSMCU_H
#define COMSMCU_H

#include "Arduino.h"
#include "ExoBLE.h"
#include "Battery.h"
#include "BleMessage.h"
#include "ParseIni.h"
#include "ExoData.h"
#include "BleMessageQueue.h"


 
#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
class ComsMCU
{
    public:
       
        ComsMCU(ExoData* data, uint8_t* config_to_send);

        void handle_ble();

        void local_sample();
        

        void update_UART();

        void update_gui();

        void handle_errors();
    private:
   
        void _life_pulse();
        const int k_pulse_count = 10;
        
        void _process_complete_gui_command(BleMessage* msg);

        //Reference to ExoBLE object, this is the next step down the composition heirarchy
        ExoBLE* _exo_ble;

        //Hold on to the last message from the GUI
        BleMessage _latest_gui_message = BleMessage();
        
        //Data
        ExoData* _data;
        
        //Battery
        _Battery* _battery;

        const int _mark_index = 1;

        //Alpha value for the exponentially weighted moving average on the battery data
        const float k_battery_ewma_alpha = 0.1;
        const float k_time_threshold = 5000; //microseconds
        
};
#endif
#endif