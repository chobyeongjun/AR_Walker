#include "Arduino.h"
#include <SD.h>
#include <SPI.h>
#include "src/Loadcell.h"
#include "src/Board.h"
#include "src/Config.h" // joint_id 정의를 위해 추가

// --- 설정 변수 ---
// BIAS와 SENSITIVITY 상수는 이제 Loadcell 클래스가 내부적으로 처리하므로 여기서는 필요 없습니다.
const int SD_CS_PIN = BUILTIN_SDCARD;
const unsigned long LOGGING_INTERVAL = 100; // 0.1초

// --- 전역 변수 ---
// 테스트할 로드셀의 ID를 정의합니다.
const config_defs::joint_id left_loadcell_id = (config_defs::joint_id)((uint8_t)config_defs::joint_id::left | (uint8_t)config_defs::joint_id::knee);
const config_defs::joint_id right_loadcell_id = (config_defs::joint_id)((uint8_t)config_defs::joint_id::right | (uint8_t)config_defs::joint_id::knee);

// Loadcell 객체 생성 시, id와 pin을 함께 전달합니다.
Loadcell left_loadcell(left_loadcell_id, logic_micro_pins::loadcell_left[0]);
Loadcell right_loadcell(right_loadcell_id, logic_micro_pins::loadcell_right[0]);

File dataFile;
unsigned long last_log_time = 0;
bool is_running = false; // 로깅 및 출력 상태 제어 플래그

// 함수 선언
void handleSerialInput();
void readAndLogData();

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("\nLoadcell Test Sketch");

  // --- SD 카드 초기화 ---
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Initialization failed!");
    while (1);
  } else {
    Serial.println("Initialization done.");
  }

  // --- 데이터 파일 생성 ---
  dataFile = SD.open("loadcelldata.csv", FILE_WRITE);
  if (dataFile) {
    Serial.println("File 'loadcelldata.csv' opened.");
    dataFile.println("Timestamp,LeftRaw,LeftCalibrated,RightRaw,RightCalibrated");
    dataFile.flush();
  } else {
    Serial.println("Error opening loadcelldata.csv");
  }

  Serial.println("----------------------------------------------------------");
  Serial.println("Press 's' to start reading/logging. Press 'e' to stop.");
}

void loop() {
  handleSerialInput();

  if (is_running) {
    readAndLogData();
  }
  
  delay(1);
}

void handleSerialInput() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 's' || command == 'S') {
      if (!is_running) {
        is_running = true;
        Serial.println("\n>>> Starting data logging and printing...\n");
        if (dataFile) dataFile.println("--- LOGGING STARTED ---");
      }
    } else if (command == 'e' || command == 'E') {
      if (is_running) {
        is_running = false;
        Serial.println("\n>>> Stopping data logging and printing...\n");
        if (dataFile) {
          dataFile.println("--- LOGGING STOPPED ---");
          dataFile.flush();
        }
      }
    }
  }
}

void readAndLogData() {
  if (millis() - last_log_time >= LOGGING_INTERVAL) {
    last_log_time = millis();

    // 파라미터 없이 read() 함수를 호출합니다.
    float left_weight = left_loadcell.read();
    float right_weight = right_loadcell.read();

    // 시리얼 모니터에 결과 출력
    Serial.print("Left  - Raw: ");
    Serial.print(left_loadcell._raw_reading);
    Serial.print("\t| Calibrated: ");
    Serial.print(left_weight);
    Serial.print("\t\tRight - Raw: ");
    Serial.print(right_loadcell._raw_reading);
    Serial.print("\t| Calibrated: ");
    Serial.println(right_weight);

    // SD 카드에 데이터 기록
    if (dataFile) {
      String dataString = String(last_log_time) + "," +
                          String(left_loadcell._raw_reading) + "," +
                          String(left_weight) + "," +
                          String(right_loadcell._raw_reading) + "," +
                          String(right_weight);
      dataFile.println(dataString);
    }
  }
}
