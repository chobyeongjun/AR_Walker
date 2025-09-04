#ifndef StatusLed_h
#define StatusLed_h

#include "Arduino.h"
#include "Board.h"
#include "StatusDefs.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#include <map>

namespace status_led_defs
{
    typedef std::map<uint16_t, uint16_t> IdxRemap; /**< Type to map an error message to a display type */

    /**< Maps messages to idx used for color and pattern. Mainly needed so errors and warnings will look the same.*/
    const IdxRemap status_led_idx = //    IdxRemap 이라는 이름의 status_led_idx
        {
            // Status messages

            {status_defs::messages::off, 0},
            {status_defs::messages::trial_off, 1},
            {status_defs::messages::trial_on, 2},
            {status_defs::messages::test, 3},
            {status_defs::messages::motor_start_up, 4},

            {status_defs::messages::error, 8},

            {status_defs::messages::error_left_knee_loadcell, 8},
            {status_defs::messages::error_right_knee_loadcell, 8},
            {status_defs::messages::error_left_ankle_loadcell, 8},
            {status_defs::messages::error_right_ankle_loadcell, 8},

            {status_defs::messages::error_left_knee_imu, 8},
            {status_defs::messages::error_right_knee_imu, 8},
            {status_defs::messages::error_left_ankle_imu, 8},
            {status_defs::messages::error_right_ankle_imu, 8},

            {status_defs::messages::error_left_knee_motor, 8},
            {status_defs::messages::error_left_ankle_motor, 8},
            {status_defs::messages::error_right_knee_motor, 8},
            {status_defs::messages::error_right_ankle_motor, 8},
            {status_defs::messages::error_left_knee_controller, 8},
            {status_defs::messages::error_left_ankle_controller, 8},
            {status_defs::messages::error_right_knee_controller, 8},
            {status_defs::messages::error_right_ankle_controller, 8},

            {status_defs::messages::error_to_be_used_1, 8},
            {status_defs::messages::error_to_be_used_2, 8},
            {status_defs::messages::error_to_be_used_3, 8},
            {status_defs::messages::error_to_be_used_4, 8},
            {status_defs::messages::error_to_be_used_5, 8},
            {status_defs::messages::error_to_be_used_6, 8},
            {status_defs::messages::error_to_be_used_7, 8},

            {status_defs::messages::warning, 9},
            {status_defs::messages::warning_exo_run_time, 9},
            {status_defs::messages::warning_to_be_used_1, 9},
            {status_defs::messages::warning_to_be_used_2, 9},
            {status_defs::messages::warning_to_be_used_3, 9},
            {status_defs::messages::warning_to_be_used_4, 9},
            {status_defs::messages::warning_to_be_used_5, 9},
            {status_defs::messages::warning_to_be_used_6, 9},
            {status_defs::messages::warning_to_be_used_7, 9},
            {status_defs::messages::warning_to_be_used_8, 9},
            {status_defs::messages::warning_to_be_used_9, 9},
            {status_defs::messages::warning_to_be_used_10, 9},
            {status_defs::messages::warning_to_be_used_11, 9},
            {status_defs::messages::warning_to_be_used_12, 9},
            {status_defs::messages::warning_to_be_used_13, 9},
            {status_defs::messages::warning_to_be_used_14, 9},
    };

}
#endif

// Define the on and off state of the LED. This is handy for if you are using a P Channel MOSFET where low is on.
// #define STATUS_LED_ON_STATE 0
// #define STATUS_LED_OFF_STATE 255

// Color assumes 255 is on. The code will use the on/off state above to compensate for the code.
// #define STATUS_MESSAGE_LED_OFF 0              //Set the message index
// #define STATUS_COLOR_LED_OFF {0, 0, 0}        //Set the color in {R, G, B} format 0-255

// #define STATUS_MESSAGE_TRIAL_OFF 1            //Set the message index
// #define STATUS_COLOR_TRIAL_OFF {0, 0, 255}    //Set the color in {R, G, B} format 0-255

// #define STATUS_MESSAGE_TRIAL_ON 2             //Set the message index
// #define STATUS_COLOR_TRIAL_ON {0, 255, 0}     //Set the color in {R, G, B} format 0-255

// #define STATUS_MESSAGE_ERROR 3                //Set the message index
// #define STATUS_COLOR_ERROR {255, 0, 0}        //Set the color in {R, G, B} format 0-255

// #define NO_PWM true //True if using simple digital pins, false if using pwm pins

namespace status_led_defs
{

    namespace colors // Just used namespace due to the complex structure.
    {
        const int off[] = {0, 0, 0};                      // black
        const int trial_off[] = {0, 0, 255};              // blue
        const int trial_on[] = {0, 255, 0};               // green
        const int test[] = {68, 255, 0};                  // 노란빛깔
        const int loadcell_calibration[] = {255, 255, 0}; // 노랑
        const int imu_start[] = {0, 255, 255};            // 청록
        const int imu_stop[] = {0, 255, 68};              // 밝은 연두
        const int motor_start_up[] = {255, 0, 68};        // 보라빛
        const int error[] = {255, 0, 0};                  // red
        const int warning[] = {255, 68, 0};               // orange
    }

    namespace patterns // Just used namespace due to the complex structure.
    {
        const uint8_t solid = 0;
        const uint8_t blink = 1;
        const uint8_t pulse = 2;
        const uint8_t rainbow = 3;

        // Format is pattern number and period in ms
        const int off[] = {solid, 0};        // Solid
        const int trial_off[] = {solid, 0};  // Solid
        const int trial_on[] = {pulse, 500}; // Pulse  // 500ms 주기로 깜빡임
        const int test[] = {rainbow, 4000};  // Rainbow
        const int loadcell_calibration[] = {solid, 500};
        const int imu_start[] = {pulse, 500};
        const int imu_stop[] = {pulse, 500};
        const int motor_start_up[] = {pulse, 4000}; // Pulse
        const int error[] = {blink, 250};           // Blinking
        const int warning[] = {blink, 250};         // Blinking
    }

    const uint8_t on_state = logic_micro_pins::status_led_on_state;   /**< On state of the LED used to determine PWM ratio*/
    const uint8_t off_state = logic_micro_pins::status_led_off_state; /**< Off state of the LED used to determine PWM ratio*/

    const bool has_pwm = logic_micro_pins::status_has_pwm; /**< Records if the display should use PWM or simple digital output. */
}

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

class StatusLed
{
public:
    // Constructors one you can set the default LED State 생성자야
    StatusLed(int r_pin, int g_pin, int b_pin);                 // Pins are the pins assocated with the different LED inputs
    StatusLed(int r_pin, int g_pin, int b_pin, int brightness); // Pins are the pins assocated with the different LED inputs, brightness is used to scale the colors that are sent: color * brightness/255

    // 가장 중요한 메소드로, status_led_defs::status_led_idx에 정의된 메시지 인덱스를 사용하여 LED 상태를 업데이트합니다.
    void update(uint16_t message); // Changes the LED State to the current state

    void set_brightness(int brightness); // Used if you need to change the brightness after initialization, brightness is used to scale the colors that are sent: color * brightness/255

    // Toggle the LED state between on and off
    // 이 메소드는 LED의 상태를 켜거나 끄는 역할을 합니다
    void toggle();

private:
    void _set_color(int R, int G, int B); // Changes the color R,G,and B are 0-255 values to set the corresponding colors.

    void _solid();

    void _pulse();

    void _blink();

    void _rainbow_sin();

    void _rainbow_hsv();

    int _r_pin; /**< Pin used for the red LED */                                                 // 핀 번호
    int _g_pin;                                                                                  /**< Pin used for the green LED */
    int _b_pin;                                                                                  /**< Pin used for the blue LED */
    int _brightness;                                                                             /**< Max brightness of LED this scales the RGB colors, color * brightness/255 */
    int _current_message; /**< Index of the current message used to select the correct color. */ // 시스템 상태 메세지랑 유형 인덱스
    uint16_t _msg_idx;                                                                           /**< Index the current message uses for display */

    // blink나 pulse 같은 패턴을 구현하기 위해 현재 시간, 주기(ms) 등을 추적하고 저장하는 변수
    int _pattern_start_timestamp;    /**< Keeps track of the time the current pattern has run. */
    int _period_ms;                  /**< Period of the pattern */
    int _pattern_brightness_percent; /**< Percent of the  for the pattern brightness. */

    // Make sure to keep in index order from messages, this is an array of the colors to use _messageColors[_currentMessage][color] where color is 0 for r, 1 for g, and 2 for b.
    // This method of accessing array elements is bulky but works.

    /**< Mapping from _msg_idx to color */
    // msg_idx

    const int _message_colors[10][3] = {{status_led_defs::colors::off[0], status_led_defs::colors::off[1], status_led_defs::colors::off[2]},
                                        {status_led_defs::colors::trial_off[0], status_led_defs::colors::trial_off[1], status_led_defs::colors::trial_off[2]},
                                        {status_led_defs::colors::trial_on[0], status_led_defs::colors::trial_on[1], status_led_defs::colors::trial_on[2]},
                                        {status_led_defs::colors::test[0], status_led_defs::colors::test[1], status_led_defs::colors::test[2]},
                                        {status_led_defs::colors::loadcell_calibration[0], status_led_defs::colors::loadcell_calibration[1], status_led_defs::colors::loadcell_calibration[2]},
                                        {status_led_defs::colors::imu_start[0], status_led_defs::colors::imu_start[1], status_led_defs::colors::imu_start[2]},
                                        {status_led_defs::colors::imu_stop[0], status_led_defs::colors::imu_stop[1], status_led_defs::colors::imu_stop[2]},
                                        {status_led_defs::colors::motor_start_up[0], status_led_defs::colors::motor_start_up[1], status_led_defs::colors::motor_start_up[2]},
                                        {status_led_defs::colors::error[0], status_led_defs::colors::error[1], status_led_defs::colors::error[2]},
                                        {status_led_defs::colors::warning[0], status_led_defs::colors::warning[1], status_led_defs::colors::warning[2]}};

    /**< Mapping from _msg_idx to pattern */
    const int _message_pattern[10][2] = {{status_led_defs::patterns::off[0], status_led_defs::patterns::off[1]},
                                         {status_led_defs::patterns::trial_off[0], status_led_defs::patterns::trial_off[1]},
                                         {status_led_defs::patterns::trial_on[0], status_led_defs::patterns::trial_on[1]},
                                         {status_led_defs::patterns::test[0], status_led_defs::patterns::test[1]},
                                         {status_led_defs::patterns::loadcell_calibration[0], status_led_defs::patterns::loadcell_calibration[1]},
                                         {status_led_defs::patterns::imu_start[0], status_led_defs::patterns::imu_start[1]},
                                         {status_led_defs::patterns::imu_stop[0], status_led_defs::patterns::imu_stop[1]},
                                         {status_led_defs::patterns::motor_start_up[0], status_led_defs::patterns::motor_start_up[1]},
                                         {status_led_defs::patterns::error[0], status_led_defs::patterns::error[1]},
                                         {status_led_defs::patterns::warning[0], status_led_defs::patterns::warning[1]}};
};
#endif
#endif
