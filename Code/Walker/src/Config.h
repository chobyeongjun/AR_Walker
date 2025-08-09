#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "LogLevels.h"
#define FIRMWARE_VERSION 0_1_0

// Available Boards that could be utilized

#define AK_Board_V0_5_1 1

// Board that will actually be used
#define BOARD_VERSION AK_Board_V0_5_1 // Note: Only include name (e.g., AK_Board_V0_5_1), no need for number (e.g., 4)

#define REAL_TIME_I2C 0
#define LOOP_FREQ_HZ 500 // main loop frequency in Hz
#define LOOP_TIME_TOLERANCE 0.1

#define USE_SPEED_CHECK 0
#define USE_ANGLE_SENSORS 0

// MACRO magic to convert a define to a string
#define VAL(str) #str
#define TOSTRING(str) VAL(str)

// Release: No prints, Debug: Prints to serial monitor
namespace logging
{
    const LogLevel level = LogLevel::Release; // Release or Debug (Note: Enter Debug to have Logger print to serial monitor)
    const int baud_rate = 115200;
}

namespace sync_time
{
    const unsigned int NUM_START_STOP_BLINKS = 1;                                // The number of times to have the LED on during the start stop sequence
    const unsigned int SYNC_HALF_PERIOD_US = 125000;                             // Half blink period in micro seconds
    const unsigned int SYNC_START_STOP_HALF_PERIOD_US = 4 * SYNC_HALF_PERIOD_US; // Half blink period for the begining and end of the sequence. This is usually longer so it is easy to identify.
}

namespace analog
{
    const float RESOLUTION = 12; // The resolution of the analog to digital converter
    const float COUNTS = 4096;   // The number of counts the ADC can have
}

namespace loadcell_calibration
{
    const float AI_CNT_TO_V = 3.3 / 4096; // Conversion from count to voltage

}

namespace BLE_times
{
    const float _status_msg_delay = 2000000;  // Microseconds
    const float _real_time_msg_delay = 14285; // Microseconds
    const float _update_delay = 1000;         // Microseconds
    const float _poll_timeout = 4;            // Milliseconds
}

// Update this namespace for future exo updates to display correct information on app
namespace exo_info
{
    const String FirmwareVersion = String(TOSTRING(FIRMWARE_VERSION)); // String to add to firmware char
    const String PCBVersion = String(TOSTRING(BOARD_VERSION));         // String to add to pcb char
    const String DeviceName = String("H-Walker");                      // String to add to device char, if you would like the system to set it use "NULL"
}

namespace UART_times
{
    const float UPDATE_PERIOD = 1000;    // Microseconds, time between updating data over uart
    const float COMS_MCU_TIMEOUT = 5000; // Microseconds
    const float CONT_MCU_TIMEOUT = 1000; // Microseconds
    const float CONFIG_TIMEOUT = 8000;   // Milliseconds
}

#endif