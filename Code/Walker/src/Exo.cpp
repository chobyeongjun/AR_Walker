#include "Exo.h"
#include "Time_Helper.h"
#include "UARTHandler.h"
#include "UART_msg_t.h"
#include "uart_commands.h"
#include "Logger.h"

#define EXO_DEBUG

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

Exo::Exo(ExoData *exo_data)
    : left_side(true, exo_data), right_side(false, exo_data), sync_led(logic_micro_pins::sync_led_pin, sync_time::SYNC_START_STOP_HALF_PERIOD_US, sync_time::SYNC_HALF_PERIOD_US, logic_micro_pins::sync_led_on_state, logic_micro_pins::sync_default_pin), status_led(logic_micro_pins::status_led_r_pin, logic_micro_pins::status_led_g_pin, logic_micro_pins::status_led_b_pin), _imu() // IMU 객체 초기화
{
    this->data = exo_data;
    pinMode(logic_micro_pins::motor_stop_pin, INPUT_PULLUP);
};

bool Exo::run()
{
    static UARTHandler *handler = UARTHandler::get_instance();
    static Time_Helper *t_helper = Time_Helper::get_instance();
    static float context = t_helper->generate_new_context();
    static float delta_t = 0;
    static uint16_t prev_status = data->get_status();
    delta_t += t_helper->tick(context);

    static float rt_context = t_helper->generate_new_context();
    static float rt_delta_t = 0;

    static const float lower_bound = (float)1 / LOOP_FREQ_HZ * 1000000 * (1 - LOOP_TIME_TOLERANCE);

    if (delta_t >= (lower_bound))
    {
        data->sync_led_state = sync_led.handler(); // <- 추가된 코드

        // E-stop 확인
        data->estop = digitalRead(logic_micro_pins::motor_stop_pin);
        if (data->estop)
        {
            left_side.disable_motors();
            right_side.disable_motors();
        }

        // IMU 데이터 읽고 SideData에 분배
        _imu.readData();
        _update_imu_data();

        // 각 측면의 센서 데이터 읽고 모터 명령 전송
        left_side.run_side();
        right_side.run_side();

        status_led.update(data->get_status());

        UART_msg_t msg = handler->poll(UART_times::CONT_MCU_TIMEOUT);
        UART_command_utils::handle_msg(handler, data, msg);

        rt_delta_t += t_helper->tick(rt_context);
        uint16_t exo_status = data->get_status();
        const bool correct_status = (exo_status == status_defs::messages::trial_on) || (exo_status == status_defs::messages::error);

        if ((rt_delta_t >= BLE_times::_real_time_msg_delay) && (correct_status))
        {
            UART_msg_t msg;
            UART_command_handlers::get_real_time_data(handler, data, msg, data->config);
            rt_delta_t = 0;
        }

        delta_t = 0;
        return true;
    }
    return false;
}

void Exo::_update_imu_data()
{
    uint8_t imu_id = _imu.get_id();

    if (imu_id == data->left_side.imu_id_knee)
    {
        data->left_side.imu_pitch = _imu.yaw;
        data->left_side.imu_gyro_y = _imu.gyro_z;
        //...
    }
    else if (imu_id == data->left_side.imu_id_ankle)
    {
        data->left_side.imu_pitch = _imu.yaw;
        data->left_side.imu_gyro_y = _imu.gyro_z;
        //...
    }
    else if (imu_id == data->right_side.imu_id_knee)
    {
        data->right_side.imu_pitch = _imu.yaw;
        data->right_side.imu_gyro_y = _imu.gyro_z;
        //...
    }
    else if (imu_id == data->right_side.imu_id_ankle)
    {
        data->right_side.imu_pitch = _imu.yaw;
        data->right_side.imu_gyro_y = _imu.gyro_z;
        //...
    }
}
#endif