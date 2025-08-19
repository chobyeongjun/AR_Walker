#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#include "ExoBLE.h"
#include "Utilities.h"
#include "Time_Helper.h"
#include "ComsLed.h"
#include "Config.h"
#include "error_codes.h"
#include "Logger.h"

#define EXOBLE_DEBUG 1

ExoBLE::ExoBLE()
{
    ;
}

bool ExoBLE::setup()
{
    if (!BLE.begin())
    {
        utils::spin_on_error_with("BLE.begin() failed");
        return false;
    }

    // Setup name and initialize data
    // BLE 어드레스에서 ':' 문자를 제거하고, 특정 길이(MAC_ADDRESS_NAME_LENGTH)만큼 잘라내어 고유한 이름 생성
    // BLE_address 구성 방식 == sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    String name = utils::remove_all_chars(BLE.address(), ':');
    // MAC 주소 문자열의 마지막 MAC_ADDRESS_NAME_LENGTH 만큼만 남깁니다.
    name.remove(name.length() - MAC_ADDRESS_NAME_LENGTH);

    // 미리 정의된 접두사
    name = NAME_PREAMBLE + name;

    // Using exo_info namespace defined in Config.h
    String FirmwareVersion = exo_info::FirmwareVersion; // String to add to firmware char
    String PCBVersion = exo_info::PCBVersion;           // String to add to pcb char
    String DeviceName = exo_info::DeviceName;           // String to add to device char

    // Check if the name is null, if it is use the name above, if not check for preamble
    if (DeviceName == "NULL")
    {
        DeviceName = name;
    }
    else
    {
        // Check if the name has the preamble, if not add it
        if (!DeviceName.startsWith(NAME_PREAMBLE))
        {
            DeviceName = NAME_PREAMBLE + DeviceName;
        }
    }

    // Initialize char arrays lengths
    // String` 객체를 `char` 배열로 변환 (BLE 통신 스택에 전달하기 위함)
    char name_char[name.length()];
    char firmware_char[FirmwareVersion.length()];
    char pcb_char[PCBVersion.length()];
    char device_char[DeviceName.length()];

    // Add data to array
    // `String` 길이에 맞는 `char` 배열을 선언합니다. `length() + 1`은 문자열의 끝을 나타내는 `null` 문자 공간을 확보하기 위함입니다.
    name.toCharArray(name_char, name.length() + 1);
    FirmwareVersion.toCharArray(firmware_char, FirmwareVersion.length() + 1);
    PCBVersion.toCharArray(pcb_char, PCBVersion.length() + 1);
    DeviceName.toCharArray(device_char, DeviceName.length() + 1);

    // `const char *` 포인터 생성 (`char` 배열의 시작 주소를 가리킴)
    // BLE 라이브러리 함수들이 `const char *` 타입을 요구하는 경우가 많기 때문에 포인터를 생성합니다.
    const char *k_name_pointer = name_char;
    const char *firmware_pointer = firmware_char;
    const char *pcb_pointer = pcb_char;
    const char *device_pointer = device_char;

    // Set name for device
    BLE.setLocalName(k_name_pointer);  // BLE 광고 패킷에 포함될 장치 이름 (주변 기기에서 검색 시 보임)
    BLE.setDeviceName(k_name_pointer); // BLE 장치의 완전한 이름 (연결 후 장치 정보 서비스에서 확인 가능)

    // Initialize GATT DB
    // `_gatt_db` 객체(GATT 서비스 및 특성들을 정의하는 구조체/클래스)의 특성들에 초기 값을 씁니다
    _gatt_db.FirmwareChar.writeValue(firmware_char); // 펌웨어 버전 특성 값 설정 (예: "v1.0")
    _gatt_db.PCBChar.writeValue(pcb_char);           // PCB 버전 특성 값 설정 (예: "v2.1")
    _gatt_db.DeviceChar.writeValue(device_char);     // 장치 이름 특성 값 설정 (예: "EXO_A1B2C3")
    send_error(0, 0);

    // Configure services and advertising data
    // --- BLE 서비스 및 광고 데이터 설정 -
    // BLE 광고 패킷에 `_gatt_db.UARTService`가 있음을 알립니다. (주변 기기가 이 서비스를 보고 장치를 특정 용도로 식별할 수 있도록)
    BLE.setAdvertisedService(_gatt_db.UARTService);

    // UART Chars
    // --- UART 서비스 특성 추가 (데이터 송수신 채널) ---
    // `_gatt_db.UARTService`에 데이터 송수신을 위한 두 가지 특성(`Characteristic`)을 추가합니다.
    _gatt_db.UARTService.addCharacteristic(_gatt_db.TXChar);
    _gatt_db.UARTService.addCharacteristic(_gatt_db.RXChar);

    // Device Info Chars
    // --- 장치 정보 서비스 특성 추가 ---
    // `_gatt_db.UARTServiceDeviceInfo`에 장치 정보를 담은 특성들을 추가합니다.
    _gatt_db.UARTServiceDeviceInfo.addCharacteristic(_gatt_db.PCBChar);
    _gatt_db.UARTServiceDeviceInfo.addCharacteristic(_gatt_db.FirmwareChar);
    _gatt_db.UARTServiceDeviceInfo.addCharacteristic(_gatt_db.DeviceChar);

    // Error Char
    // --- 오류 서비스 특성 추가 ---
    // `_gatt_db.ErrorService`에 오류 정보를 담은 특성을 추가합니다.
    _gatt_db.ErrorService.addCharacteristic(_gatt_db.ErrorChar);

    // --- 정의된 모든 BLE 서비스 등록 (GATT 서버에 서비스들을 최종적으로 추가) ---
    BLE.addService(_gatt_db.UARTService);
    BLE.addService(_gatt_db.UARTServiceDeviceInfo);
    BLE.addService(_gatt_db.ErrorService);

    // ---  RX 특성 이벤트 핸들러 설정 (데이터 수신 시 자동 호출될 함수 지정) ---
    // 외부 기기가 `RXChar`에 데이터를 쓰면 (`BLEWritten` 이벤트), `ble_rx::on_rx_recieved` 함수가 자동으로 호출되도록 설정합니다.
    _gatt_db.RXChar.setEventHandler(BLEWritten, ble_rx::on_rx_recieved);

    // --- BLE 연결 간격 설정 ---
    // 최소/최대 연결 간격을 6으로 설정합니다. (BLE는 1.25ms 단위이므로, 6은 7.5ms를 의미합니다. 빠른 데이터 교환에 유리)
    BLE.setConnectionInterval(6, 6);
    advertising_onoff(true);

    return true;
}

void ExoBLE::advertising_onoff(bool onoff)
{
    if (onoff) // `onoff`가 참(true)이면 (광고를 시작해야 할 경우)
    {
        // Start Advertising
        //  logger::println("Start Advertising"); BLE 모듈에게 "광고를 시작해라" 명령
        BLE.advertise();

        // Turn the blue led off
        ComsLed *led = ComsLed::get_instance();
        uint8_t r, g, b;
        led->get_color(&r, &g, &b);
        led->set_color(r, g, 0); // 이는 장치가 '광고 중' 또는 '연결 대기 중'임을 시각적으로 나타내는 상태 표시로 보입니다.
    }
    else
    {
        // Stop Advertising
        //  logger::println("Stop Advertising");
        BLE.stopAdvertise();

        // Turn the blue led on
        ComsLed *led = ComsLed::get_instance();
        uint8_t r, g, b;
        led->get_color(&r, &g, &b);
        led->set_color(r, g, 255);
    }
}

bool ExoBLE::handle_updates()
{
#if EXOBLE_DEBUG
    logger::print("ExoBLE::handle_updates:Start");
    logger::print("\n");
#endif

    static Time_Helper *t_helper = Time_Helper::get_instance();
    static float update_context = t_helper->generate_new_context();
    static float del_t = 0;
    del_t += t_helper->tick(update_context); // tick 으로 경과 시간 누적 계산

    // del_t` (누적 시간)이 `BLE_times::_update_delay`
    if (del_t > BLE_times::_update_delay)
    {
        del_t = 0; // 누적 시간을 0으로 리셋하여 다음 주기를 다시 계산합니다.
#if EXOBLE_DEBUG
        static float poll_context = t_helper->generate_new_context();
        static float poll_time = 0;
        static float connected_context = t_helper->generate_new_context();
        static float connected_time = 0;
#endif

// Poll for updates and check connection status
#if EXOBLE_DEBUG
        logger::print("Poll for updates and check connection status");
        logger::print("\n");
#endif

        BLE.poll();
        ;                                         // BLE 통신 스택의 내부 이벤트(데이터 수신, 연결/해제 등)를 처리하고 상태를 업데이트합니다.
        int32_t current_status = BLE.connected(); // 현재 BLE 연결 상태

        if (_connected == current_status) // 이전 연결 상태(`_connected`)와 현재 상태(`current_status`)가 동일하다면 (상태 변화 없음)
        {
#if EXOBLE_DEBUG
            logger::print("ExoBLE::handle_updates:queue size:");
            logger::print(ble_queue::size());
            logger::print("\n");
#endif

            return ble_queue::size(); // `BleMessageQueue`에 처리할 메시지가 몇 개 있는지 반환합니다. (메시지 처리 여부 신호)
        }

        // The BLE connection status changed
        if (current_status < _connected) // 연결이 끊어진 경우
        {
// Disconnection
#if EXOBLE_DEBUG
            logger::print("Disconnection");
            logger::print("\n");
#endif
        }
        else if (current_status > _connected) // 연결이 성립된 경우
        {
// Connection
#if EXOBLE_DEBUG
            logger::print("Connection");
            logger::print("\n");
#endif
        }

        advertising_onoff(current_status == 0);
        _connected = current_status;
    }

#if EXOBLE_DEBUG
    logger::print("ExoBLE::handle_updates:queue size:");
    logger::print(ble_queue::size());
    logger::print("\n");
#endif

    return ble_queue::size();
}

void ExoBLE::send_message(BleMessage &msg)
{
    if (!this->_connected) // BLE 중앙 장치(예: 스마트폰)에 연결되어 있지 않다면
    {
        return; /* Don't bother sending anything if no one is listening */
    }

#if EXOBLE_DEBUG
    BleMessage::print(msg);
#endif

    static const int k_preamble_length = 3; // 메시지 페이로드(내용)가 차지할 수 있는 최대 `char` 배열 길이를 계산합니다.
    int max_payload_length = ((k_preamble_length + msg.expecting) * (MAX_PARSER_CHARACTERS + 1));
    byte buffer[max_payload_length]; // 전송할 데이터를 담을 임시 버퍼를 스택에 할당합니다.

    // `_ble_parser` 객체(원시 데이터 <-> BleMessage 변환 담당)의 `package_raw_data()` 함수를 호출하여,
    // 보낼 `BleMessage`(`msg`)를 BLE 전송에 적합한 원시 바이트 형태로 `buffer`에 채워 넣습니다.
    // `bytes_to_send` 변수에는 실제로 `buffer`에 채워진 데이터의 길이가 반환
    int bytes_to_send = _ble_parser.package_raw_data(buffer, msg);

    // `_gatt_db.TXChar` 특성(Characteristic)에 변환된 `buffer`의 데이터를 `bytes_to_send` 길이만큼 씁니다.
    // 이 작업은 BLE 스택을 통해 무선으로 데이터가 외부 장치로 전송되도록 합니다.
    _gatt_db.TXChar.writeValue(buffer, bytes_to_send);
}

void ExoBLE::send_error(int error_code, int joint_id)
{
    if (!this->_connected)
    {
        return; /* Don't bother sending anything if no one is listening */
    }

#if EXOBLE_DEBUG
    logger::print("Exoble::send_error->Sending: ", LogLevel::Error);
    logger::print(joint_id, LogLevel::Error);
    logger::print(", ", LogLevel::Error);
    logger::print(error_code, LogLevel::Error);
    logger::print("\n");
#endif
    // `error_code`와 `joint_id`를 콜론(`:`)으로 구분하여 하나의 `String` 문자열을 만듭니다. (예: "101:5" -> 오류 코드 101, 관절 ID 5)

    String error_string = String(error_code) + ":" + String(joint_id);

    // Convert to char array // --- `String`을 `char` 배열로 변환 ---
    char error_char[error_string.length() + 1];
    error_string.toCharArray(error_char, error_string.length() + 1);

    _gatt_db.ErrorChar.writeValue(error_char);
}

void ble_rx::on_rx_recieved(BLEDevice central, BLECharacteristic characteristic)
{
    static BleMessage *empty_msg = new BleMessage();
    static BleParser *parser = new BleParser();
    static BleMessage *msg = new BleMessage();

    // Must reset message to avoid duplicate data   // `msg` 객체를 `empty_msg`의 내용으로 초기화합니다.
    (*msg) = *empty_msg;

    char data[32] = {0};                    // 수신된 데이터를 임시로 담을 `char` 배열
    int len = characteristic.valueLength(); // 수신된 데이터의 실제 길이
    characteristic.readValue(data, len);    // `characteristic`으로부터 실제 데이터를 `data` 배열로 읽어옵니다.

#if EXOBLE_DEBUG
    logger::print("On Rx Recieved: ");
    for (int i = 0; i < len; i++)
    {
        logger::print(data[i]);
        logger::print(", ");
    }
    logger::print("\n");
#endif

    // `parser` 객체의 `handle_raw_data()` 함수를 호출하여 수신된 원시 데이터(`data`, `len`)를 `BleMessage` 객체(`msg`)로 해석합니다.
    // `handle_raw_data`는 여러 BLE 패킷에 걸쳐 메시지가 올 경우 이를 모아주는 역할을 합니다.
    msg = parser->handle_raw_data(data, len);

    if (msg->is_complete) // `parser->handle_raw_data`가 반환한 `msg`가 완전한 메시지라면
    {
#if EXOBLE_DEBUG
        logger::print("on_rx_recieved->Command: ");
        BleMessage::print(*msg);
#endif

        ble_queue::push(msg); // 완성된 `BleMessage`를 `BleMessageQueue`에 추가합니다.
                              // 이렇게 하면 메시지는 큐에 안전하게 저장되고, 메인 루프의 `ComsMCU::handle_ble()`과 같은 함수가
                              // 큐에서 메시지를 꺼내어 순서대로 처리할 수 있게 됩니다.
    }

#if EXOBLE_DEBUG
    logger::print("on_rx_recieved->End\n");
#endif
}

#endif // defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)