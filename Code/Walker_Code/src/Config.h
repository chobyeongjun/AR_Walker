/**
 * @file Config.h
 * @author Chancelor Cuddeback
 * @brief Configuration variables for the codebase.
 * @date 2023-07-18
 *
 */

#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include "LogLevels.h"
#define FIRMWARE_VERSION 0_1_0

// Available Boards that could be utilized
#define AK_Board_V0_5_1 1

// Board that will actually be used
#define BOARD_VERSION AK_Board_V0_5_1 // Note: Only include name (e.g., AK_Board_V0_5_1), no need for number (e.g., 4)

#define REAL_TIME_I2C 1
#define LOOP_FREQ_HZ 500 // main loop frequency in Hz
#define LOOP_TIME_TOLERANCE 0.1

#define USE_SPEED_CHECK 0
#define USE_ANGLE_SENSORS 0

// MACRO magic to convert a define to a string
#define VAL(str) #str
#define TOSTRING(str) VAL(str)

// Logging settings 여기에서 우리가 배포를 할 지 Debug를 할 지 결정한다.
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

// Angle 을 아날로그로 받아와서 Threshold 설정한 거니까 그거는 삭제 해도 될듯
namespace angle_sensor
{
    const float ANGLE_UPPER_THRESHOLD = 0.9;
    const float ANGLE_LOWER_THRESHOLD = 0.1;
    const float ROM_LEFT = 103.2f;
    const float ROM_RIGHT = 91.4f; // In degrees
}

// Analog to Digital Converter (ADC) settings
// Note: Teensy 3.6 and 4.1 have a 12-bit ADC, which means the values range from 0 to 4095

namespace analog
{
    const float RESOLUTION = 12; // The resolution of the analog to digital converter
    const float COUNTS = 4096;   // The number of counts the ADC can have
}

// Torque Calibration => 우리는 로드셀 캘리브레이션을 해야함.
namespace torque_calibration
{ // 마이크로컨트롤러의 ADC가 읽어들인 디지털 카운트 값(CNT)을 전압(V)으로 변환하는 계수 4096은 12비트 ADC의 해상도(2^12 = 4096)를 의미할 가능성이 높습니다.
    // 1 카운트당 몇 볼트인지, 우리가 계수를 찾던가 다른 방법을 해야할 듯.
    const float AI_CNT_TO_V = 3.3 / 4096; // Conversion from count to voltage
    const float TRQ_V_TO_NM = 53.70;      // Calibration factor from voltage to torque in Nm/V
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