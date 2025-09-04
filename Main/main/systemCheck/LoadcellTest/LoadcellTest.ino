/*
 * MediWalker Loadcell Test & Calibration System with SD Card Logging
 * 
 * 이 코드는 MediWalker 시스템의 로드셀 센서를 테스트하고 캘리브레이션하기 위한 
 * 독립적인 테스트 프로그램입니다.
 * 
 * 기능:
 * - 로드셀 Raw 데이터 및 캘리브레이션된 데이터 실시간 출력
 * - 각 관절별(왼쪽/오른쪽 무릎/발목) 로드셀 개별 테스트  
 * - 시리얼 모니터를 통한 캘리브레이션 파라미터 조정
 * - 센서 연결 상태 확인
 * - SD카드를 통한 데이터 로깅 (CSV 형식: time, left_voltage, left_force, right_voltage, right_force)
 * 
 * 하드웨어:
 * - Teensy 4.1
 * - 로드셀: A16 (Left), A6 (Right) 핀 연결
 * - SD카드: 내장 SD카드 슬롯 사용
 * 
 * 연속 모드: 설정된 간격(100ms)마다 자동으로 센서 데이터를 읽고 출력하는 모드
 */

#include "Arduino.h"
#include "SD.h"
#include "SPI.h"

// Teensy 4.1 SD카드 핀 설정
#define SD_CS_PIN BUILTIN_SDCARD

// Board Configuration
#define BOARD_VERSION 1  // AK_Board_V0_5_1
#define ARDUINO_TEENSY41

// 로드셀 핀 정의 (Board.h에서 가져온 정보)
namespace loadcell_pins {
    const unsigned int not_connected_pin = 51;
    const unsigned int left_pin = A16;   // 왼쪽 로드셀 (A16)
    const unsigned int right_pin = A6;   // 오른쪽 로드셀 (A6)
}

// 캘리브레이션 상수 (Config.h에서 가져온 정보)
namespace calibration {
    const float AI_CNT_TO_V = 3.3 / 4096;  // ADC 카운트를 전압으로 변환
    
    // 기본 캘리브레이션 값들 (테스트용)
    float left_knee_bias = -682.8357f;
    float left_knee_sensitive = 560.7490f;
    float right_knee_bias = -654.4153f;
    float right_knee_sensitive = 541.1179f;
    
    float left_ankle_bias = 0.0f;
    float left_ankle_sensitive = 1.0f;
    float right_ankle_bias = 0.0f;
    float right_ankle_sensitive = 1.0f;
}

// 관절 ID 정의 (ParseIni.h에서 가져온 정보)
enum class joint_id : uint8_t {
    left = 0b01000000,
    right = 0b00100000,
    knee = 0b00000001,
    ankle = 0b00000010,
    left_knee = left | knee,      // 65
    left_ankle = left | ankle,    // 66
    right_knee = right | knee,    // 33
    right_ankle = right | ankle   // 34
};

// 로드셀 클래스 (Loadcell.h/cpp 기반으로 단순화)
class TestLoadcell {
private:
    joint_id _id;
    bool _is_used;
    bool _is_left;
    unsigned int _pin;
    int _raw_reading;
    float _calibrated_weight;
    float _voltage;
    
public:
    TestLoadcell(joint_id id, unsigned int pin) {
        _id = id;
        _is_left = ((uint8_t)_id & (uint8_t)joint_id::left) == (uint8_t)joint_id::left;
        _is_used = (pin != loadcell_pins::not_connected_pin);
        _pin = pin;
        _raw_reading = 0;
        _calibrated_weight = 0;
        _voltage = 0.0f;
        
        if (_is_used) {
            pinMode(_pin, INPUT);
        }
    }
    
    float read() {
        if (!_is_used) return 0.0f;
        
        float bias = 0.0f;
        float sensitive = 1.0f;
        
        // 관절 종류와 좌우 구분하여 캘리브레이션 값 선택
        uint8_t joint_type = (uint8_t)_id & (~(uint8_t)joint_id::left & ~(uint8_t)joint_id::right);
        
        switch (joint_type) {
            case (uint8_t)joint_id::knee:
                if (_is_left) {
                    bias = calibration::left_knee_bias;
                    sensitive = calibration::left_knee_sensitive;
                } else {
                    bias = calibration::right_knee_bias;
                    sensitive = calibration::right_knee_sensitive;
                }
                break;
                
            case (uint8_t)joint_id::ankle:
                if (_is_left) {
                    bias = calibration::left_ankle_bias;
                    sensitive = calibration::left_ankle_sensitive;
                } else {
                    bias = calibration::right_ankle_bias;
                    sensitive = calibration::right_ankle_sensitive;
                }
                break;
        }
        
        _raw_reading = analogRead(_pin);
        _voltage = (float)_raw_reading * calibration::AI_CNT_TO_V;
        _calibrated_weight = (_voltage * sensitive + bias);
        return _calibrated_weight;
    }
    
    int readRaw() {
        if (_is_used) {
            return analogRead(_pin);
        }
        return 0;
    }
    
    // Getter 함수들
    bool isUsed() { return _is_used; }
    bool isLeft() { return _is_left; }
    joint_id getId() { return _id; }
    int getRawReading() { return _raw_reading; }
    float getCalibratedWeight() { return _calibrated_weight; }
    float getVoltage() { return _voltage; }
    unsigned int getPin() { return _pin; }
};

// 전역 변수
TestLoadcell left_loadcell(joint_id::left_knee, loadcell_pins::left_pin);    // 왼쪽 로드셀 (무릎으로 테스트)
TestLoadcell right_loadcell(joint_id::right_knee, loadcell_pins::right_pin); // 오른쪽 로드셀 (무릎으로 테스트)

unsigned long last_print_time = 0;
unsigned long last_log_time = 0;
const unsigned long print_interval = 100;  // 100ms마다 출력
const unsigned long log_interval = 50;     // 50ms마다 SD카드 로깅 (더 높은 해상도)

bool continuous_mode = true;
int test_mode = 0;  // 0: 연속모드, 1: 왼쪽만, 2: 오른쪽만, 3: 캘리브레이션 모드
bool sd_logging = false;
bool sd_available = false;

File dataFile;
String current_filename = "";

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        // 시리얼 연결 대기 (최대 3초)
    }
    
    Serial.println("=================================================");
    Serial.println("  MediWalker Loadcell Test & Calibration v2.0");
    Serial.println("           with SD Card Data Logging");
    Serial.println("=================================================");
    Serial.println();
    
    // SD카드 초기화
    initializeSD();
    
    // 로드셀 연결 상태 확인
    Serial.println("로드셀 연결 상태 확인:");
    Serial.print("왼쪽 로드셀 (Pin A");
    Serial.print(loadcell_pins::left_pin - A0);
    Serial.print("): ");
    Serial.println(left_loadcell.isUsed() ? "연결됨" : "연결 안됨");
    
    Serial.print("오른쪽 로드셀 (Pin A");
    Serial.print(loadcell_pins::right_pin - A0);  
    Serial.print("): ");
    Serial.println(right_loadcell.isUsed() ? "연결됨" : "연결 안됨");
    Serial.println();
    
    // 캘리브레이션 값 출력
    printCalibrationValues();
    
    printMenu();
    
    // ADC 해상도 설정 (Teensy 4.1은 기본 12bit)
    analogReadResolution(12);
}

void loop() {
    // 시리얼 명령 처리
    if (Serial.available() > 0) {
        processSerialCommand();
    }
    
    // 데이터 출력 (지정된 간격마다)
    if (millis() - last_print_time >= print_interval) {
        printLoadcellData();
        last_print_time = millis();
    }
    
    // SD카드 로깅 (더 높은 빈도)
    if (sd_logging && sd_available && (millis() - last_log_time >= log_interval)) {
        logDataToSD();
        last_log_time = millis();
    }
}

void initializeSD() {
    Serial.print("SD카드 초기화 중...");
    
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println(" 실패!");
        Serial.println("SD카드가 없거나 오류가 발생했습니다.");
        sd_available = false;
        return;
    }
    
    Serial.println(" 성공!");
    sd_available = true;
    

}

String generateFilename() {
    // 현재 시간 기반 파일명 생성 (millis 기반)
    unsigned long currentTime = millis();
    return "loadcell_" + String(currentTime / 1000) + ".csv";
}

void startDataLogging() {
    if (!sd_available) {
        Serial.println("SD카드를 사용할 수 없습니다!");
        return;
    }
    
    current_filename = generateFilename();
    dataFile = SD.open(current_filename.c_str(), FILE_WRITE);
    
    if (dataFile) {
        // CSV 헤더 작성
        dataFile.println("time_ms,left_voltage,left_force,right_voltage,right_force");
        dataFile.flush();
        sd_logging = true;
        Serial.print("데이터 로깅 시작: ");
        Serial.println(current_filename);
    } else {
        Serial.println("파일을 열 수 없습니다!");
    }
}

void stopDataLogging() {
    if (sd_logging && dataFile) {
        dataFile.close();
        sd_logging = false;
        Serial.print("데이터 로깅 완료: ");
        Serial.println(current_filename);
        Serial.print("파일 크기: ");
        
        File file = SD.open(current_filename.c_str());
        if (file) {
            Serial.print(file.size());
            Serial.println(" bytes");
            file.close();
        }
    }
}

void logDataToSD() {
    if (!dataFile) return;
    
    // 데이터 읽기
    float left_force = left_loadcell.read();
    float left_voltage = left_loadcell.getVoltage();
    
    float right_force = right_loadcell.read();
    float right_voltage = right_loadcell.getVoltage();
    
    // CSV 형식으로 데이터 저장
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(left_voltage, 4);
    dataFile.print(",");
    dataFile.print(left_force, 4);
    dataFile.print(",");
    dataFile.print(right_voltage, 4);
    dataFile.print(",");
    dataFile.println(right_force, 4);
    
    // 주기적으로 플러시 (데이터 손실 방지)
    static int flush_counter = 0;
    if (++flush_counter >= 20) {  // 20개 데이터마다 플러시 (약 1초마다)
        dataFile.flush();
        flush_counter = 0;
    }
}

void printMenu() {
    Serial.println("명령어:");
    Serial.println("  'a' - 연속 모드 (모든 로드셀, 100ms 간격으로 자동 출력)");
    Serial.println("  'l' - 왼쪽 로드셀만 테스트");
    Serial.println("  'r' - 오른쪽 로드셀만 테스트");
    Serial.println("  'c' - 캘리브레이션 모드");
    Serial.println("  's' - 정지");
    Serial.println("  'h' - 도움말 (이 메뉴)");
    Serial.println("  'z' - 영점 조정 (현재 값을 0으로 설정)");
    Serial.println("  'p' - 4점 선형 캘리브레이션 (0N 포함)");
    Serial.println("  --- SD카드 로깅 ---");
    Serial.println("  'g' - 데이터 로깅 시작 (CSV 파일로 저장)");
    Serial.println("  't' - 데이터 로깅 정지");
    Serial.println("  'f' - SD카드 파일 목록 보기");
    Serial.println("=================================================");
}

void printCalibrationValues() {
    Serial.println("현재 캘리브레이션 값:");
    Serial.print("왼쪽 무릎  - Bias: ");
    Serial.print(calibration::left_knee_bias);
    Serial.print(", Sensitive: ");
    Serial.println(calibration::left_knee_sensitive);
    
    Serial.print("오른쪽 무릎 - Bias: ");
    Serial.print(calibration::right_knee_bias);
    Serial.print(", Sensitive: ");
    Serial.println(calibration::right_knee_sensitive);
    Serial.println();
}

void processSerialCommand() {
    char command = Serial.read();
    
    switch (command) {
        case 'a':
        case 'A':
            test_mode = 0;
            continuous_mode = true;
            Serial.println("연속 모드 시작 - 모든 로드셀 (100ms 간격)");
            break;
            
        case 'l':
        case 'L':
            test_mode = 1;
            continuous_mode = true;
            Serial.println("왼쪽 로드셀만 테스트");
            break;
            
        case 'r':
        case 'R':
            test_mode = 2;
            continuous_mode = true;
            Serial.println("오른쪽 로드셀만 테스트");
            break;
            
        case 'c':
        case 'C':
            test_mode = 3;
            continuous_mode = false;
            enterCalibrationMode();
            break;
            
        case 's':
        case 'S':
            continuous_mode = false;
            if (sd_logging) stopDataLogging();
            Serial.println("테스트 정지");
            break;
            
        case 'h':
        case 'H':
            printMenu();
            break;
            
        case 'z':
        case 'Z':
            zeroCalibration();
            break;
            
        case 'g':
        case 'G':
            startDataLogging();
            break;
            
        case 't':
        case 'T':
            stopDataLogging();
            break;
            
        case 'f':
        case 'F':
            listSDFiles();
            break;
        
            
        default:
            // 알 수 없는 명령은 무시
            break;
    }
}

void printLoadcellData() {
    if (!continuous_mode) return;
    
    switch (test_mode) {
        case 0:  // 연속 모드 (모든 로드셀)
            printAllLoadcells();
            break;
        case 1:  // 왼쪽만
            printSingleLoadcell(left_loadcell, "왼쪽");
            break;
        case 2:  // 오른쪽만
            printSingleLoadcell(right_loadcell, "오른쪽");
            break;
    }
}

void printAllLoadcells() {
    // 왼쪽 로드셀 데이터
    float left_force = left_loadcell.read();
    float left_voltage = left_loadcell.getVoltage();
    int left_raw = left_loadcell.getRawReading();
    
    // 오른쪽 로드셀 데이터  
    float right_force = right_loadcell.read();
    float right_voltage = right_loadcell.getVoltage();
    int right_raw = right_loadcell.getRawReading();
    
    // 헤더 (20줄마다)
    static int line_count = 0;
    if (line_count % 20 == 0) {
        Serial.println("시간(ms) | L_Raw | L_V(V) | L_F(N) | R_Raw | R_V(V) | R_F(N) | SD로그");
        Serial.println("---------|-------|--------|--------|-------|--------|--------|---------");
    }
    line_count++;
    
    // 데이터 출력
    Serial.print(millis());
    Serial.print("\t| ");
    Serial.print(left_raw);
    Serial.print("\t| ");
    Serial.print(left_voltage, 3);
    Serial.print("\t| ");
    Serial.print(left_force, 2);
    Serial.print("\t| ");
    Serial.print(right_raw);
    Serial.print("\t| ");
    Serial.print(right_voltage, 3);
    Serial.print("\t| ");
    Serial.print(right_force, 2);
    Serial.print("\t| ");
    Serial.println(sd_logging ? "ON" : "OFF");
}

void printSingleLoadcell(TestLoadcell& loadcell, const char* name) {
    float force = loadcell.read();
    float voltage = loadcell.getVoltage();
    int raw = loadcell.getRawReading();
    
    Serial.print(name);
    Serial.print(" - Raw: ");
    Serial.print(raw);
    Serial.print(", 전압: ");
    Serial.print(voltage, 3);
    Serial.print("V, 힘: ");
    Serial.print(force, 2);
    Serial.print("N");
    Serial.print(" [SD로그: ");
    Serial.print(sd_logging ? "ON" : "OFF");
    Serial.println("]");
}

void zeroCalibration() {
    Serial.println("영점 조정 중...");
    
    // 현재 값들을 읽어서 bias로 설정
    left_loadcell.read();
    right_loadcell.read();
    
    float left_voltage = left_loadcell.getVoltage();
    float right_voltage = right_loadcell.getVoltage();
    
    // 현재 전압을 0이 되도록 bias 조정
    calibration::left_knee_bias = -left_voltage * calibration::left_knee_sensitive;
    calibration::right_knee_bias = -right_voltage * calibration::right_knee_sensitive;
    
    Serial.println("영점 조정 완료!");
    Serial.print("왼쪽 새로운 Bias: ");
    Serial.println(calibration::left_knee_bias);
    Serial.print("오른쪽 새로운 Bias: ");
    Serial.println(calibration::right_knee_bias);
}

void enterCalibrationMode() {
    Serial.println("=== 캘리브레이션 모드 ===");
    printCalibrationValues();
    Serial.println("수정할 값의 번호를 입력하세요 (1-4), 또는 'q'로 종료:");
    
    while (true) {
        if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            input.trim();
            
            if (input == "q" || input == "Q") {
                Serial.println("캘리브레이션 모드 종료");
                return;
            }
            
            int selection = input.toInt();
            if (selection >= 1 && selection <= 4) {
                adjustCalibrationValue(selection);
                Serial.println("다른 값을 수정하려면 1-4를 입력하세요. 종료하려면 'q':");
            } else {
                Serial.println("잘못된 입력입니다. 1-4 또는 'q'를 입력하세요:");
            }
        }
        delay(50);
    }
}

void adjustCalibrationValue(int selection) {
    Serial.print("새로운 값을 입력하세요: ");
    
    // 입력 대기
    while (Serial.available() == 0) {
        delay(50);
    }
    
    float new_value = Serial.parseFloat();
    
    switch (selection) {
        case 1:
            calibration::left_knee_bias = new_value;
            Serial.print("왼쪽 Bias를 ");
            Serial.print(new_value);
            Serial.println("로 설정했습니다.");
            break;
        case 2:
            calibration::left_knee_sensitive = new_value;
            Serial.print("왼쪽 Sensitive를 ");
            Serial.print(new_value);
            Serial.println("로 설정했습니다.");
            break;
        case 3:
            calibration::right_knee_bias = new_value;
            Serial.print("오른쪽 Bias를 ");
            Serial.print(new_value);
            Serial.println("로 설정했습니다.");
            break;
        case 4:
            calibration::right_knee_sensitive = new_value;
            Serial.print("오른쪽 Sensitive를 ");
            Serial.print(new_value);
            Serial.println("로 설정했습니다.");
            break;
    }
}

void listSDFiles() {
    if (!sd_available) {
        Serial.println("SD카드를 사용할 수 없습니다!");
        return;
    }
    
    Serial.println("=== SD카드 파일 목록 ===");
    File root = SD.open("/");
    int file_count = 0;
    
    while (true) {
        File entry = root.openNextFile();
        if (!entry) {
            break;
        }
        
        if (!entry.isDirectory()) {
            Serial.print(entry.name());
            Serial.print("\t\t");
            Serial.print(entry.size());
            Serial.println(" bytes");
            file_count++;
        }
        entry.close();
    }
    root.close();
    
    Serial.print("총 ");
    Serial.print(file_count);
    Serial.println("개 파일");
    Serial.println("========================");
}