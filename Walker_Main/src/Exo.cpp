#include "Exo.h"
#include "Time_Helper.h"
#include "UARTHandler.h"
#include "UART_msg_t.h"
#include "uart_commands.h"
#include "Logger.h"

#define EXO_DEBUG

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

Exo::Exo(ExoData *exo_data)
    : _left_ankle_imu(0),  
      _right_ankle_imu(1), 
      _left_knee_imu(2),   
      _right_knee_imu(3),  
      left_side(true, exo_data), right_side(false, exo_data), 
      sync_led(logic_micro_pins::sync_led_pin, sync_time::SYNC_START_STOP_HALF_PERIOD_US, sync_time::SYNC_HALF_PERIOD_US, logic_micro_pins::sync_led_on_state, logic_micro_pins::sync_default_pin), 
      status_led(logic_micro_pins::status_led_r_pin, logic_micro_pins::status_led_g_pin, logic_micro_pins::status_led_b_pin)
{
    this->data = exo_data;
    pinMode(logic_micro_pins::motor_stop_pin, INPUT_PULLUP);
    
    // IMU 시스템 초기화
    _initialize_imu_system();
    
#ifdef EXO_DEBUG
    logger::println("Exo::Exo -> IMU system initialized");
#endif
};

void Exo::_process_imu_data()
{
    while (Serial4.available()) {
        uint8_t byte = Serial4.read();

        // SOP 바이트를 찾아서 패킷의 시작점을 동기화
        if (buffer_index == 0 && byte != IMU_SOP_BYTE) { 
            continue; 
        }
        if (buffer_index == 1 && byte != IMU_SOP_BYTE) { 
            buffer_index = 0; 
            continue; 
        }
        
        // 버퍼 오버플로우 방지
        if (buffer_index >= MAX_BUFFER_SIZE) {
            _reset_imu_buffer();
            continue;
        }
        
        receive_buffer[buffer_index++] = byte;

        if (buffer_index == TOTAL_PACKET_SIZE) {
            // IMU ID 미리 확인 (패킷 검증은 IMU.read_packet()에서 처리)
            uint8_t imu_id = receive_buffer[3];
            if (imu_id >= 4) {
                _reset_imu_buffer();
                continue;
            }
            
            // 패킷 카운트 증가
            imu_packet_count[imu_id]++;
            
            bool data_processed = false;
            
            // IMU별 데이터 처리 (검증은 read_packet()에서 자동 처리)
            if (imu_id == _left_ankle_imu.IMU_id) {
                if (_left_ankle_imu.read_packet(receive_buffer, TOTAL_PACKET_SIZE)) {
                    this->data->left_side.ankle.imu_pitch = _left_ankle_imu.pitch;
                    this->data->left_side.ankle.imu_gyro_y = _left_ankle_imu.gyro_y;
                    this->data->left_side.ankle.imu_battery = _left_ankle_imu.battery;
                    imu_status[imu_id] = IMU_Status::CONNECTED;
                    last_imu_data_time[imu_id] = millis();
                    data_processed = true;
                }
            } else if (imu_id == _right_ankle_imu.IMU_id) {
                if (_right_ankle_imu.read_packet(receive_buffer, TOTAL_PACKET_SIZE)) {
                    this->data->right_side.ankle.imu_pitch = _right_ankle_imu.pitch;
                    this->data->right_side.ankle.imu_gyro_y = _right_ankle_imu.gyro_y;
                    this->data->right_side.ankle.imu_battery = _right_ankle_imu.battery;
                    imu_status[imu_id] = IMU_Status::CONNECTED;
                    last_imu_data_time[imu_id] = millis();
                    data_processed = true;
                }
            } else if (imu_id == _left_knee_imu.IMU_id) {
                if (_left_knee_imu.read_packet(receive_buffer, TOTAL_PACKET_SIZE)) {
                    this->data->left_side.knee.imu_pitch = _left_knee_imu.pitch;
                    this->data->left_side.knee.imu_gyro_y = _left_knee_imu.gyro_y;
                    this->data->left_side.knee.imu_battery = _left_knee_imu.battery;
                    imu_status[imu_id] = IMU_Status::CONNECTED;
                    last_imu_data_time[imu_id] = millis();
                    data_processed = true;
                }
            } else if (imu_id == _right_knee_imu.IMU_id) {
                if (_right_knee_imu.read_packet(receive_buffer, TOTAL_PACKET_SIZE)) {
                    this->data->right_side.knee.imu_pitch = _right_knee_imu.pitch;
                    this->data->right_side.knee.imu_gyro_y = _right_knee_imu.gyro_y;
                    this->data->right_side.knee.imu_battery = _right_knee_imu.battery;
                    imu_status[imu_id] = IMU_Status::CONNECTED;
                    last_imu_data_time[imu_id] = millis();
                    data_processed = true;
                }
            }
            
            // 데이터 처리 실패 시 에러 처리 (read_packet()이 false 반환)
            if (!data_processed) {
                _handle_imu_error(imu_id);
#ifdef EXO_DEBUG
                logger::println("IMU " + String(imu_id) + " read_packet() failed");
#endif
            }
            
            _reset_imu_buffer();
        }
    }
    
    // 주기적으로 상태 업데이트
    static unsigned long last_status_update = 0;
    if (millis() - last_status_update > 1000) {  // 1초마다
        _update_imu_status();
        last_status_update = millis();
        
#ifdef EXO_DEBUG
        static unsigned long last_diagnostic_print = 0;
        if (millis() - last_diagnostic_print > 10000) {  // 10초마다 진단 정보 출력
            _print_imu_diagnostics();
            last_diagnostic_print = millis();
        }
#endif
    }
}

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
        data->sync_led_state = sync_led.handler();
        if (data->estop)
        {
            left_side.disable_motors();
            right_side.disable_motors();
        }

        // 💡 IMU 데이터 처리 함수 호출
        _process_imu_data();

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

// =============================================================================
// IMU System Implementation
// =============================================================================

void Exo::_initialize_imu_system()
{
    // 버퍼 초기화
    buffer_index = 0;
    memset(receive_buffer, 0, MAX_BUFFER_SIZE);
    
    // 상태 변수 초기화
    left_ankle_available = false;
    right_ankle_available = false;
    left_knee_available = false;
    right_knee_available = false;
    
    // 시간 및 카운터 초기화
    for (int i = 0; i < 4; i++) {
        last_imu_data_time[i] = 0;
        imu_error_count[i] = 0;
        imu_packet_count[i] = 0;
        imu_status[i] = IMU_Status::INITIALIZING;
    }
    
    // Serial4 초기화 (IMU 통신용 - RX 전용)
    Serial4.begin(115200);
    
    // ⚠️ IMU는 TX 전용이므로 초기화 명령 전송하지 않음
    // IMU 객체들은 데이터 처리용으로만 사용
    // _left_ankle_imu.begin()  <- 호출하지 않음 (TX 명령 포함 가능)
    // _right_ankle_imu.begin() <- 호출하지 않음
    // _left_knee_imu.begin()   <- 호출하지 않음  
    // _right_knee_imu.begin()  <- 호출하지 않음
    
#ifdef EXO_DEBUG
    logger::println("IMU system initialized - Serial4 RX-only mode ready");
    logger::println("Warning: No IMU initialization commands sent (TX-only IMUs)");
#endif
}

void Exo::_update_imu_status()
{
    unsigned long current_time = millis();
    
    for (int i = 0; i < 4; i++) {
        // 타임아웃 체크
        if (current_time - last_imu_data_time[i] > IMU_TIMEOUT_MS) {
            if (imu_status[i] == IMU_Status::CONNECTED) {
                imu_status[i] = IMU_Status::TIMEOUT;
#ifdef EXO_DEBUG
                logger::println("IMU " + String(i) + " timeout");
#endif
            }
        }
        
        // 에러율 체크
        if (imu_packet_count[i] > 10) {  // 최소 10개 패킷 받은 후 체크
            float error_rate = (float)imu_error_count[i] / imu_packet_count[i];
            if (error_rate > 0.1) {  // 10% 이상 에러율
                imu_status[i] = IMU_Status::ERROR;
            }
        }
    }
    
    // 상태별 플래그 업데이트
    left_ankle_available = (imu_status[0] == IMU_Status::CONNECTED);
    right_ankle_available = (imu_status[1] == IMU_Status::CONNECTED);
    left_knee_available = (imu_status[2] == IMU_Status::CONNECTED);
    right_knee_available = (imu_status[3] == IMU_Status::CONNECTED);
}

bool Exo::_is_imu_connected(uint8_t imu_id)
{
    if (imu_id >= 4) return false;
    return (imu_status[imu_id] == IMU_Status::CONNECTED);
}

void Exo::_reset_imu_buffer()
{
    buffer_index = 0;
    memset(receive_buffer, 0, MAX_BUFFER_SIZE);
}

Exo::IMU_Status Exo::_get_imu_status(uint8_t imu_id)
{
    if (imu_id >= 4) return IMU_Status::DISCONNECTED;
    return imu_status[imu_id];
}

void Exo::_handle_imu_error(uint8_t imu_id)
{
    if (imu_id >= 4) return;
    
    imu_error_count[imu_id]++;
    
#ifdef EXO_DEBUG
    logger::println("IMU " + String(imu_id) + " error count: " + String(imu_error_count[imu_id]));
#endif
    
    // 에러가 너무 많으면 상태 변경
    if (imu_error_count[imu_id] > MAX_IMU_ERRORS) {
        imu_status[imu_id] = IMU_Status::ERROR;
    }
}

void Exo::_reset_imu_counters(uint8_t imu_id)
{
    if (imu_id >= 4) return;
    
    imu_error_count[imu_id] = 0;
    imu_packet_count[imu_id] = 0;
    imu_status[imu_id] = IMU_Status::INITIALIZING;
    
#ifdef EXO_DEBUG
    logger::println("IMU " + String(imu_id) + " counters reset");
#endif
}

float Exo::_get_imu_packet_success_rate(uint8_t imu_id)
{
    if (imu_id >= 4 || imu_packet_count[imu_id] == 0) return 0.0f;
    
    return (float)(imu_packet_count[imu_id] - imu_error_count[imu_id]) / imu_packet_count[imu_id] * 100.0f;
}

void Exo::_print_imu_diagnostics()
{
#ifdef EXO_DEBUG
    logger::println("=== IMU Diagnostics ===");
    
    const char* status_names[] = {"DISC", "CONN", "TOUT", "ERR", "INIT"};
    const char* imu_names[] = {"L_Ankle", "R_Ankle", "L_Knee", "R_Knee"};
    
    for (int i = 0; i < 4; i++) {
        logger::print(String(imu_names[i]) + ": ");
        logger::print(String(status_names[(int)imu_status[i]]) + " ");
        logger::print("Pkts:" + String(imu_packet_count[i]) + " ");
        logger::print("Errs:" + String(imu_error_count[i]) + " ");
        logger::println("Rate:" + String(_get_imu_packet_success_rate(i)) + "%");
    }
    logger::println("=======================");
#endif
}

bool Exo::_validate_imu_packet(uint8_t* packet)
{
    if (packet == nullptr) return false;
    
    // SOP 바이트 체크
    if (packet[0] != IMU_SOP_BYTE || packet[1] != IMU_SOP_BYTE) {
        return false;
    }
    
    // 패킷 길이 체크
    if (packet[2] != (TOTAL_PACKET_SIZE - 3)) {
        return false;
    }
    
    // SUM 체크섬 검증 (IMU.cpp와 동일한 방식)
    const int DATA_LENGTH_FOR_CHECKSUM = TOTAL_PACKET_SIZE - 2;
    uint16_t checksum = 0;
    for (int i = 0; i < DATA_LENGTH_FOR_CHECKSUM; i++) {
        checksum += packet[i];
    }
    
    // 수신된 체크섬 (마지막 2바이트, big-endian)
    uint16_t received_checksum;
    memcpy(&received_checksum, &packet[TOTAL_PACKET_SIZE - 2], 2);
    uint16_t imu_checksum = __builtin_bswap16(received_checksum);
    
    return (imu_checksum == checksum);
}

uint8_t Exo::_calculate_checksum(uint8_t* packet, size_t length)
{
    // 이 함수는 더 이상 사용되지 않음 (SUM 체크섬으로 변경)
    // 하지만 호환성을 위해 유지
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum ^= packet[i];  // XOR 체크섬
    }
    return checksum;
}

#endif