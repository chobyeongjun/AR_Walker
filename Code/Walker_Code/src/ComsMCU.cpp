#include "ComsMCU.h"
#include "StatusLed.h"
#include "StatusDefs.h"
#include "Time_Helper.h"
#include "UARTHandler.h"
#include "uart_commands.h"
#include "UART_msg_t.h"
#include "Config.h"
#include "error_codes.h"
#include "Logger.h"
#include "ComsLed.h"

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#define COMSMCU_DEBUG 0

ComsMCU::ComsMCU(ExoData *data, uint8_t *config_to_send) : _data{data}
{
    switch (config_to_send[config_defs::battery_idx])
    {
    case (uint8_t)config_defs::battery::smart:
        _battery = new SmartBattery();
        break;
    case (uint8_t)config_defs::battery::dumb:
        _battery = new RCBattery();
        break;
    default:
        // logger::println("ERROR: ComsMCU::ComsMCU->Unrecognized battery type!");
        _battery = new RCBattery();
        break;
    }

    _battery->init();        // 선택된 배터리 객체 초기화
    _exo_ble = new ExoBLE(); // ExoBLE 객체 생성
    _exo_ble->setup();       // BLE 통신 설정 (이름, 서비스, 특성 등)

    // 3. 외골격(exo) 이름에 따른 실시간 데이터 길이 설정
    uint8_t rt_data_len = 0;
    switch (config_to_send[config_defs::exo_name_idx])
    {
    case (uint8_t)config_defs::exo_name::bilateral_ankle:
        rt_data_len = rt_data::BILATERAL_ANKLE_RT_LEN;
        break;
    case (uint8_t)config_defs::exo_name::bilateral_hip:
        rt_data_len = rt_data::BILATERAL_HIP_RT_LEN;
        break;
    case (uint8_t)config_defs::exo_name::bilateral_hip_ankle:
        rt_data_len = rt_data::BILATERAL_HIP_ANKLE_RT_LEN;
        break;
    default:
        rt_data_len = 8;
        break;
    }

    // rt_data::msg_len = rt_data_len
    //  logger::print("ComsMCU::ComsMCU->rt_data_len: "); logger::println(rt_data_len);
}

void ComsMCU::handle_ble()
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::handle_ble->Start");
#endif
    // BLE 업데이트를 처리하고, 큐에 새로운 메시지가 있는지 확인
    bool non_empty_ble_queue = _exo_ble->handle_updates();

    if (non_empty_ble_queue) // BLE 큐에 처리할 메시지가 있다면
    {
#if COMSMCU_DEBUG
        logger::println("ComsMCU::handle_ble->non_empty_ble_queue");
#endif

        BleMessage msg = ble_queue::pop();   // 큐에서 가장 오래된 메시지(명령)를 꺼냄
        _process_complete_gui_command(&msg); // 꺼낸 명령 메시지 처리

#if COMSMCU_DEBUG
        logger::println("ComsMCU::handle_ble->processed message");
#endif
    }

#if COMSMCU_DEBUG
    logger::println("ComsMCU::handle_ble->End");
#endif
}

void ComsMCU::local_sample()
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::local_sample->Start");
#endif

    Time_Helper *t_helper = Time_Helper::get_instance();

    // 시간 측정용 컨텍스트 생성
    static const float context = t_helper->generate_new_context();
    static float del_t = 0;
    del_t += t_helper->tick(context); // 이전 호출 이후 시간 누적

    if (del_t > (BLE_times::_status_msg_delay / 2)) // 특정 시간 간격마다 (상태 메시지 딜레이의 절반)
    {
        static float filtered_value = _battery->get_parameter();                               // 필터링된 배터리 값 저장 변수
        float raw_battery_value = _battery->get_parameter();                                   // 배터리 원시 값 읽기
        filtered_value = utils::ewma(raw_battery_value, filtered_value, k_battery_ewma_alpha); // 지수 이동 평균 필터 적용
        _data->battery_value = filtered_value;                                                 // 필터링된 배터리 값을 _data에 저장
        del_t = 0;                                                                             // 시간 누적 초기화
    }

    ComsLed::get_instance()->life_pulse(); // 통신 LED 깜빡임

#if COMSMCU_DEBUG
    logger::println("ComsMCU::local_sample->End");
#endif
}

void ComsMCU::update_UART()
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::update_UART->Start");
#endif

    static Time_Helper *t_helper = Time_Helper::get_instance();
    static const float _context = t_helper->generate_new_context(); // 시간 측정용 컨텍스트
    static float del_t = 0;
    del_t += t_helper->tick(_context); // 이전 호출 이후 시간 누적

    if (del_t > UART_times::UPDATE_PERIOD) // UART 통신 업데이트 주기가 되면
    {
        UARTHandler *handler = UARTHandler::get_instance();
        UART_msg_t msg = handler->poll(UART_times::COMS_MCU_TIMEOUT); // UART로부터 메시지 폴링

        if (msg.command) // 유효한 UART 메시지를 받았다면
        {
            UART_command_utils::handle_msg(handler, _data, msg); // UART 메시지 처리
        }

        del_t = 0; // 시간 누적 초기화
    }

#if COMSMCU_DEBUG
    logger::println("ComsMCU::update_UART->End");
#endif
}

void ComsMCU::update_gui() // GUI에 실시간 데이터를 업데이트합니다.
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::update_gui->Start");
#endif

    static Time_Helper *t_helper = Time_Helper::get_instance();
    static float my_mark = _data->mark;                // _data에서 '마크' 값 가져오기
    static float *rt_floats = new float(rt_data::len); // 실시간 데이터를 저장할 동적 배열 (길이 rt_data::len)

    // Get real time data from ExoData and send to GUI
    //  실시간 데이터(real_time_i2c)를 폴링하여 새로운 데이터가 있는지 확인
    const bool new_rt_data = real_time_i2c::poll(rt_floats);
    static float del_t_no_msg = millis(); // 메시지 없음을 추적하는 타이머

    if (new_rt_data || rt_data::new_rt_msg) // 새로운 실시간 데이터가 있거나, 새 메시지 플래그가 설정된 경우
    {
        del_t_no_msg = millis(); // 마지막 메시지 수신 시간 업데이트

#if COMSMCU_DEBUG
        logger::println("ComsMCU::update_gui->new_rt_data");
#endif

        _life_pulse();               // 내부적으로 다른 LED 깜빡임 함수 호출
        rt_data::new_rt_msg = false; // 새 메시지 플래그 리셋

        BleMessage rt_data_msg = BleMessage();                // 새 BLE 메시지 객체 생성
        rt_data_msg.command = ble_names::send_real_time_data; // 명령: 실시간 데이터 전송
        rt_data_msg.expecting = rt_data::len;                 // 예상 데이터 길이: 실시간 데이터 항목 수

        for (int i = 0; i < rt_data::len; i++) // 실시간 데이터를 메시지의 data 배열에 복사
        {
#if REAL_TIME_I2C
            rt_data_msg.data[i] = rt_floats[i];
#else
            rt_data_msg.data[i] = rt_data::float_values[i];
#endif
        }

        if (my_mark < _data->mark) // '마크' 값이 변경되었는지 확인
        {
            my_mark = _data->mark;                   // 새로운 마크 값 업데이트
            rt_data_msg.data[_mark_index] = my_mark; // 메시지의 특정 위치에 마크 값 추가
        }

        _exo_ble->send_message(rt_data_msg); // BLE를 통해 GUI로 메시지 전송

#if COMSMCU_DEBUG
        logger::println("ComsMCU::update_gui->sent message");
#endif
    }
    else // GUI로부터 메시지를 받지 못했을 때의 오류 처리 (주석 처리됨)
    {
        // If we should be getting messages and we dont for 1 second, spin on error
        uint16_t exo_status = _data->get_status();
        // trial on , fsr_calibration, fsr_refinement, error
        const bool correct_status = (exo_status == status_defs::messages::trial_on) ||
                                    (exo_status == status_defs::messages::fsr_calibration) ||
                                    (exo_status == status_defs::messages::fsr_refinement) ||
                                    (exo_status == status_defs::messages::error);

        if (correct_status)
        {
            // if (millis() - del_t_no_msg > 3000)
            // {
            //     #if COMSMCU_DEBUG
            //          logger::println("ComsMCU::update_gui->No message for 3 second");
            //     #endif
            //     while (true)
            //     {
            //         logger::println("ComsMCU::update_gui->No message for 3 second");
            //         delay(10000);
            //     }
            // }
        }
    }

    // Periodically send status information
    static float status_context = t_helper->generate_new_context(); // 상태 전송용 컨텍스트
    static float del_t_status = 0;
    del_t_status += t_helper->tick(status_context);  // 이전 호출 이후 시간 누적
    if (del_t_status > BLE_times::_status_msg_delay) // 상태 메시지 전송 주기가 되면
    {
#if COMSMCU_DEBUG
        logger::println("ComsMCU::update_gui->Sending status");
#endif

        // Send status data
        BleMessage batt_msg = BleMessage();                                                 // 배터리 메시지 생성
        batt_msg.command = ble_names::send_batt;                                            // 명령: 배터리 상태 전송
        batt_msg.expecting = ble_command_helpers::get_length_for_command(batt_msg.command); // 명령에 따른 예상 길이
        batt_msg.data[0] = _data->battery_value;                                            // 배터리 값 설정
        _exo_ble->send_message(batt_msg);                                                   // BLE를 통해 GUI로 배터리 메시지 전송

        del_t_status = 0; // 시간 누적 초기화

#if COMSMCU_DEBUG
        logger::println("ComsMCU::update_gui->sent message");
#endif
    }

#if COMSMCU_DEBUG
    logger::println("ComsMCU::update_gui->End");
#endif
}

void ComsMCU::handle_errors()
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::handle_errors->Start");
#endif

    static ErrorCodes error_code = NO_ERROR;

    if (_data->error_code != static_cast<int>(error_code))
    {
        error_code = static_cast<ErrorCodes>(_data->error_code);
        _exo_ble->send_error(_data->error_code, _data->error_joint_id);
    }

#if COMSMCU_DEBUG
    logger::println("ComsMCU::handle_errors->End");
#endif
}

// handle_ble()에서 큐에서 꺼낸 완성된 GUI 명령 메시지(BleMessage)를 실제로 처리하는 역할
void ComsMCU::_process_complete_gui_command(BleMessage *msg)
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::_process_complete_gui_command->Start");
    BleMessage::print(*msg);
#endif

    switch (msg->command) // 블루투스로 쏘는 데이터들.
    {
    case ble_names::start:
        ble_handlers::start(_data, msg);
        break;
    case ble_names::stop:
        ble_handlers::stop(_data, msg);
        break;
    case ble_names::cal_trq:
        ble_handlers::cal_trq(_data, msg);
        break;
    case ble_names::cal_fsr:
        ble_handlers::cal_fsr(_data, msg);
        break;
    case ble_names::assist:
        ble_handlers::assist(_data, msg);
        break;
    case ble_names::resist:
        ble_handlers::resist(_data, msg);
        break;
    case ble_names::motors_on:
        ble_handlers::motors_on(_data, msg);
        break;
    case ble_names::motors_off:
        ble_handlers::motors_off(_data, msg);
        break;
    case ble_names::mark:
        ble_handlers::mark(_data, msg);
        break;
    case ble_names::new_fsr:
        ble_handlers::new_fsr(_data, msg);
        break;
    case ble_names::new_trq:
        ble_handlers::new_trq(_data, msg);
        break;
    case ble_names::update_param:
        ble_handlers::update_param(_data, msg);
        break;
    default:
        logger::println("ComsMCU::_process_complete_gui_command->No case for command!", LogLevel::Error);
        break;
    }

#if COMSMCU_DEBUG
    logger::println("ComsMCU::_process_complete_gui_command->End");
#endif
}

void ComsMCU::_life_pulse()
{
#if COMSMCU_DEBUG
    logger::println("ComsMCU::_life_pulse->Start");
#endif

    static int count = 0;
    count++;

    if (count > k_pulse_count)
    {
        count = 0;
        digitalWrite(25, !digitalRead(25)); // 핀 25)을 제어하고 다른 주기를 가질 수 있습니다.
    }

#if COMSMCU_DEBUG
    logger::println("ComsMCU::_life_pulse->End");
#endif
}
#endif