#ifndef TIME_HELPER_H
#define TIME_HELPER_H

#define MAX_TICKERS 15

#include <vector>

typedef struct
{
    float context;       // 어떤 코드 블록에 대한 시간인지"를 구분해 주는 것이 바로 컨텍스트
    float old_time = -1; // 해당 컨텍스트에 대해 tick()이 이전에 호출되었을 때의 타임스탬프를 저장
    int k_index;
} ticker_t; // timer를 만드는

class Time_Helper
{
public:
    Time_Helper(bool use_micros = true);
    static Time_Helper& get_instance();

    float peek(float context); // old_time을 재설정하지 않고 주어진 컨텍스트에 대한 현재 경과 시간을 가져올 수 있도록 합니다.
    float tick(float context); //  경과 시간을 측정하기 위한 주요 함수

    float generate_new_context(); // context 생성
    void destroy_context(float context);

private:
    bool _context_conflicts(float context);        // 주어진 context 값을 사용하여 ticker_t 구조체가 이미 존재하는지 확인하는 함수
    ticker_t *_ticker_from_context(float context); // 주어진 context 값을 사용하여 이미 존재하는 ticker_t 구조체를 찾아서 그 주소(포인터)를 반환하는 함수

    int ticker_count = 0;
    std::vector<ticker_t> tickers;

    bool _k_use_micros;
};

#endif