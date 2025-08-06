#include "ComsLed.h"
#include "Board.h"

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

ComsLed::ComsLed() // 생성자로 정의되어 있는 핀들의 색을 정의, 초기화
{
    // Set the rgb pins to output
    pinMode(coms_micro_pins::red, OUTPUT);
    pinMode(coms_micro_pins::green, OUTPUT);
    pinMode(coms_micro_pins::blue, OUTPUT);

    // Set the initial color to 0
    this->r = 0;
    this->g = 0;
    this->b = 0; // LED를 처음에는 완전히 끈 상태로 설정
    digitalWrite(coms_micro_pins::red, (bool)r);
    digitalWrite(coms_micro_pins::green, (bool)g);
    digitalWrite(coms_micro_pins::blue, (bool)b);
} // digitalWrite(...): 실제로 해당 핀에 0 값을 써서 LED를 끕니다. (여기서 (bool)r과 같이 bool로 캐스팅하는 것은
  // digitalWrite가 HIGH나 LOW를 기대하기 때문인데, 0은 false로 변환되어 LOW가 됩니다.)

ComsLed *ComsLed::get_instance()
{
    static ComsLed *instance = new ComsLed();
    return instance;
}

// r, g, b 값(각각 0부터 255까지의 밝기)을 받아서 LED의 색상을 설정
void ComsLed::set_color(uint8_t r, uint8_t g, uint8_t b)
{ // 현재 객체의 내부 색상 변수를 업데이트
    this->r = r;
    this->g = g;
    this->b = b;
    digitalWrite(coms_micro_pins::red, (bool)_handle_active_low(r));
    digitalWrite(coms_micro_pins::green, (bool)_handle_active_low(g));
    digitalWrite(coms_micro_pins::blue, (bool)_handle_active_low(b));
}

void ComsLed::get_color(uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = this->r;
    *g = this->g;
    *b = this->b;
}
//"보드가 살아있다고 깜빡깜빡 신호를 보내주세요!
// loop() 함수와 같은 주기적인 메인 루프에서 호출
// life_pulse_frequency 보다 커지면 불이 꺼졌다가 다시 켜지고
void ComsLed::life_pulse()
{
    life_pulse_counter++;
    if (life_pulse_counter > life_pulse_frequency)
    {
        life_pulse_counter = 0;

        // Flip the state of the green and blue pins
        digitalWrite(coms_micro_pins::green, !digitalRead(coms_micro_pins::green));
        digitalWrite(coms_micro_pins::blue, !digitalRead(coms_micro_pins::blue));
    }
}

// "LED 연결 방식이 Active Low 방식이면 반대로 처리해서 0 값이면 255로 처리하고 255면 0으로 처리함.
uint8_t ComsLed::_handle_active_low(uint8_t value)
{
    if (coms_micro_pins::led_active_low)
    {
        return 255 - value;
    }
    else
    {
        return value;
    }
}

#endif