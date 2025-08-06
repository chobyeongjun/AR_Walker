/*
 *
 * P. Stegall Jan. 2022
 */

#include "Exo.h"
#include "Time_Helper.h"
#include "UARTHandler.h"
#include "UART_msg_t.h"
#include "uart_commands.h"
#include "Logger.h"

// #define EXO_DEBUG  //Uncomment if you want the debug statements to print to serial monitor 이 매크로가 정의 되면 디버그 메시지가 시리얼 모니터에 출력됩니다

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
/*
 * Constructor for the Exo
 * Takes the exo_data
 * Uses initializer list for sides.
 * Only stores these objects, and exo_data pointer.
 */
Exo::Exo(ExoData *exo_data)
    : left_side(true, exo_data) // Constructor: uses initializer list for the sides
      ,
      right_side(false, exo_data) // Constructor: uses initializer list for the sides
      // Create a sync LED object, the first and last arguments (pin) are found in Board.h, and the rest are in Config.h. If you do not have a digital input for the default state you can remove SYNC_DEFAULT_STATE_PIN.
      ,
      sync_led(logic_micro_pins::sync_led_pin, sync_time::SYNC_START_STOP_HALF_PERIOD_US, sync_time::SYNC_HALF_PERIOD_US, logic_micro_pins::sync_led_on_state, logic_micro_pins::sync_default_pin), status_led(logic_micro_pins::status_led_r_pin, logic_micro_pins::status_led_g_pin, logic_micro_pins::status_led_b_pin) // Create the status LED object.
// Side 객체 초기화, SyncLED 객체 초기화, StatusLED 객체 초기화

#ifdef USE_SPEED_CHECK
      ,
      speed_check(logic_micro_pins::speed_check_pin)
#endif

{
    this->data = exo_data; // 여기서 exo_data 포인터를 클래스 멤버 'data'에 저장합니다

#ifdef EXO_DEBUG
    logger::println("Exo :: Constructor : _data set");
#endif

    pinMode(logic_micro_pins::motor_stop_pin, INPUT_PULLUP);

#ifdef EXO_DEBUG
    logger::println("Exo :: Constructor : motor_stop_pin Mode set");
#endif
};

/*
 * Run the exo
 */
bool Exo::run() // run() 메서드는 다음과 같은 논리
{
    // Check if we are within the system frequency we want.
    static UARTHandler *handler = UARTHandler::get_instance();  // 싱글톤 구조 : run()이 처음 호출될 때 단 한 번만
    static Time_Helper *t_helper = Time_Helper::get_instance(); // 싱글톤 구조 : run()이 처음 호출될 때 단 한 번만
    static float context = t_helper->generate_new_context();    // 이 함수는 현재 시간을 기반으로 context 값을 생성할 것으로 예상

    static float delta_t = 0; // run()이 처음 호출될 때 0으로 초기화된 후, 이후 호출에서는 이전 호출에서 누적된 값을 유지
    static uint16_t prev_status = data->get_status();
    delta_t += t_helper->tick(context); //  현재 루프 실행 간의 시간 간격을 누적

    // Check if the real time data is ready to be sent.
    static float rt_context = t_helper->generate_new_context();
    static float rt_delta_t = 0;

    static const float lower_bound = (float)1 / LOOP_FREQ_HZ * 1000000 * (1 - LOOP_TIME_TOLERANCE); // LOOP_FREQ_HZ: 이 값은 메인 제어 루프가 **초당 몇 번 실행되기를 원하는지(Hz)
                                                                                                    // LOOP_TIME_TOLERANCE: 이 값은 루프 실행 시간의 허용 오차를 나타내며, 예를 들어 0.1이면 10%의 오차를 허용
    if (delta_t >= (lower_bound))                                                                   // 시스템이 미리 정의된 루프 주파수(예: 100Hz)에 맞춰 한 주기의 작업을 수행할 때가 되었다
    {
#if USE_SPEED_CHECK
        logger::print(String(delta_t) + "\n");
        speed_check.toggle();
#endif

        // Check if we should update the sync LED and record the LED on/off state.
        //  data->sync_led_state = ...: sync_led.handler()가 반환하는 LED의 현재 on/off 상태를 ExoData 구조체의 sync_led_state 멤버에 저장
        //  bool trial_running = sync_led.get_is_blinking();: SyncLED 객체에서 현재 LED가 깜박이는 상태인지 (즉, 실험/시험이 진행 중인지) 여부를 가져와 trial_running 변수에 저장
        data->sync_led_state = sync_led.handler();
        bool trial_running = sync_led.get_is_blinking();

        // Check the estop  비상 정지(E-stop) 확인
        // 일반적으로 E-stop 버튼은 눌렀을 때 LOW 신호를 보냅니다
        data->estop = digitalRead(logic_micro_pins::motor_stop_pin);

        // If the estop is low, disable all of the motors
        if (data->estop)
        {
            data->for_each_joint([](JointData *j_data, float *args)
                                 { j_data->motor.enabled = false; });
        }

        // Record the side data and send new commands to the motors.
        //  LEFT RIGHT 모든 센서 값을 받아오는 로직
        left_side.run_side();
        right_side.run_side();

        // Update status LED
        status_led.update(data->get_status());
// 현재 시스템 상태(data->get_status())를 가져와서, 그 상태에 따라 RGB LED의 색상이나 깜박임 패턴을 변경하여 사용자에게 현재 시스템 상태(예: 정상, 오류, 캘리브레이션 중 등)를 시각적으로
#ifdef EXO_DEBUG
        logger::println("Exo::Run:Time_OK");
        logger::println(delta_t);
        logger::println(((float)1 / LOOP_FREQ_HZ * 1000000 * (1 + LOOP_TIME_TOLERANCE)));
#endif

        // Check for incoming UART messages
        UART_msg_t msg = handler->poll(UART_times::CONT_MCU_TIMEOUT); // UART_times::CONT_MCU_TIMEOUT is in Config.h
        UART_command_utils::handle_msg(handler, data, msg);

        // 실시간 데이터 전송을 위한 타이머 rt_delta_t를 업데이트, 이 타이머는 메인 루프 타이머와는 독립적으로 작동
        // Send the coms mcu the real time data every _real_time_msg_delay microseconds
        rt_delta_t += t_helper->tick(rt_context);
        uint16_t exo_status = data->get_status(); // 현재 외골격의 전반적인 상태
        const bool correct_status = (exo_status == status_defs::messages::trial_on) || (exo_status == status_defs::messages::fsr_calibration) || (exo_status == status_defs::messages::fsr_refinement) || (exo_status == status_defs::messages::error);
        // _real_time_msg_delay 실시간 데이터 전송 주기)보다 rt_delta_t가 크거나 같고, correct_status가 true일 때, 실시간 데이터 전송을 수행

        if ((rt_delta_t >= BLE_times::_real_time_msg_delay) && (correct_status))
        {
#ifdef EXO_DEBUG
            logger::print("Exo::run->Sending Real Time Message: ");
            logger::println(rt_delta_t);
#endif

            UART_msg_t msg;
            UART_command_handlers::get_real_time_data(handler, data, msg, data->config); // 현재 외골격의 실시간 데이터를 다른 통신 MCU로 보냅니다.
            rt_delta_t = 0;                                                              // rt_delta_t = 0;: 실시간 데이터 전송이 완료되면 타이머를 재설정하여 다음 전송 주기를 시작합니다.
        }

        delta_t = 0;
        return true;
    }

    return false;
};

#endif