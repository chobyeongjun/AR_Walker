#include "Time_Helper.h"
#include "Logger.h"
#include <Arduino.h>

/* Public */
Time_Helper::Time_Helper(bool use_micros)
{
    _k_use_micros = use_micros;
}

Time_Helper& Time_Helper::get_instance()
{
    static Time_Helper instance;  // 메모리 누수 해결: 스택 객체 사용
    return instance;              // 참조 반환
}

float Time_Helper::peek(float context)
{
    float new_time = ((_k_use_micros) ? (micros()) : (millis()));

    ticker_t *ticker = _ticker_from_context(context);

    // The context does not exist or this is the tickers first tick
    if (ticker->k_index < 0 || ticker->old_time < 0)
    {
        return 0;
    }

    return new_time - ticker->old_time; // 현재 시간(new_time)에서 ticker_t에 저장된 이전 시간(old_time)을 빼서 경과 시간을 계산
}

float Time_Helper::tick(float context)
{
    float new_time;
    if (_k_use_micros)
    {
        new_time = micros();
    }
    else
    {
        new_time = millis();
    }

    ticker_t *ticker = _ticker_from_context(context);

    // The context does not exist or this is the tickers first tick
    if (ticker->k_index < 0 || ticker->old_time < 0)
    {
        return 0;
    }

    float return_time = new_time - ticker->old_time;
    ticker->old_time = new_time; // 현재 시간을 old_time에 저장하여 다음 tick 호출시 이 시간부터 다시 계산하도록 준비
    return return_time;
}

float Time_Helper::generate_new_context()
{
    if (ticker_count == (MAX_TICKERS - 1))
    {
        return 0;
    }
    bool searching = true;
    float found;
    while (searching)
    {
        float new_context = random(1000);
        if (!_context_conflicts(new_context))
        {
            found = new_context;
            searching = false;
        }
    }

    // Track new ticker instance - 메모리 안전하게 수정
    ticker_t new_ticker;            // 스택 객체로 생성 (메모리 누수 해결)
    new_ticker.context = found;
    new_ticker.old_time = 0;
    new_ticker.k_index = ticker_count; // 현재 벡터의 크기와 같은 인덱스

    tickers.push_back(new_ticker);     // 직접 push (복사 생성)

    ticker_count++; // 새로운 타이머 추가되었으니 카운트 증가 (내가 추가)

    return found;
}

void Time_Helper::destroy_context(float context)
{
    ticker_t *ticker_to_destroy = _ticker_from_context(context);
    
    // 범위 검사 추가 (음수 인덱스 방지)
    if (ticker_to_destroy->k_index >= 0 && ticker_to_destroy->k_index < tickers.size()) {
        tickers.erase(tickers.begin() + ticker_to_destroy->k_index);  // -1 제거
        ticker_count--;
        
        // 삭제 후 인덱스 재정렬 (삭제된 위치 뒤의 모든 ticker 인덱스 -1)
        for (int i = ticker_to_destroy->k_index; i < tickers.size(); i++) {
            tickers[i].k_index = i;  // 새로운 인덱스로 업데이트
        }
    }
}

/* Private */
bool Time_Helper::_context_conflicts(float context)
{
    for (int i = 0; i < tickers.size(); i++)
    {
        if (context == tickers[i].context)
        {
            return true; // 중복하면 true 반환
        }
    }
    return false;
}

ticker_t *Time_Helper::_ticker_from_context(float context)
{
    static ticker_t err_ticker = {
        .context = 0,
        .old_time = -1,
        .k_index = -1};
    for (int i = 0; i < tickers.size(); i++)
    {
        if (context == tickers[i].context)
        {
            return &tickers[i];
        }
    }
    return &err_ticker;
}