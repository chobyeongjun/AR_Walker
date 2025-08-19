#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// 필요한 헤더 파일들을 포함
#include "src\Board.h"
#include "src\Loadcell.h"
#include "src\Logger.h"

// 로드셀 객체들: 왼쪽과 오른쪽 다리에 각각 하나씩 사용
Loadcell left_loadcell(logic_micro_pins::loadcell_left[0]);
Loadcell right_loadcell(logic_micro_pins::loadcell_right[0]);

void setup()
{
    Serial.begin(115200);
    while(!Serial)
    {
      ;
    }

    // 시리얼 모니터에 출력될 헤더
    logger::print("Left Loadcell\t");
    logger::print("Right Loadcell");
    logger::print("\n");
}

void loop()
{
    // 1ms 주기로 데이터를 읽고 출력
    int state_period_ms = 1;
    static int last_transition_time = millis();
    int current_time = millis();
    
    if ((current_time - last_transition_time) >= state_period_ms)
    {
      // 각 로드셀에서 원시(raw) 값을 읽습니다.
      float left_val = left_loadcell.readRaw();
      float right_val = right_loadcell.readRaw();
      
      last_transition_time = current_time;

      // 시리얼 모니터에 값 출력
      logger::print(left_val);
      logger::print("\t");
      logger::print(right_val);
      logger::print("\n");
    }
}
#endif