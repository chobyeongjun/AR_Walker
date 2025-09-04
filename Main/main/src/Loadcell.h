#ifndef LOADCELL_H
#define LOADCELL_H

#include "Arduino.h"
#include "Board.h"
#include "Logger.h"
#include "Config.h"
#include "ParseIni.h"


class Loadcell
{
public:
    // 생성자에서 어떤 관절인지 id를 받도록 수정
    Loadcell(config_defs::joint_id id, unsigned int pin);
    
    // read 함수에서 bias, sensitive 파라미터 삭제
    float read();

    int readRaw();

    // 이 변수들은 read()가 호출될 때마다 업데이트됩니다.
    int _raw_reading;
    float _calibrated_weight;

private:
    config_defs::joint_id _id; // 이 로드셀의 관절 ID를 저장
    bool _is_used;
    bool _is_left;
    unsigned int _pin;
};

#endif // LOADCELL_H
