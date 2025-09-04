#ifndef Exo_h
#define Exo_h

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"
#include "Side.h"
#include <stdint.h>
#include "ParseIni.h"
#include "Board.h"
#include "Utilities.h"
#include "SyncLed.h"
#include "StatusLed.h"
#include "StatusDefs.h"
#include "Config.h"
#include "ExoData.h"
#include "IMU.h"
#include "Loadcell.h"

class Exo
{
public:
    Exo(ExoData *exo_data); // Constructor: uses initializer list for the Side objects.

    bool run();

    ExoData *data;  
    Side left_side;  
    Side right_side; 

    SyncLed sync_led;     
    StatusLed status_led; 
    
    bool is_collecting_data = false;

private:
    IMU _left_ankle_imu;   
    IMU _right_ankle_imu; 
    IMU _left_knee_imu;   
    IMU _right_knee_imu;
    
    // 💡 IMU 데이터 처리를 위한 멤버 변수들을 private으로 선언
    const int TOTAL_PACKET_SIZE = 26; // IMU 프로토콜에 맞게 26바이트로 수정
    uint8_t receive_buffer[26];
    size_t buffer_index = 0;
    bool left_available = false;
    bool right_available = false;

    void _process_imu_data();

};
#endif

#endif