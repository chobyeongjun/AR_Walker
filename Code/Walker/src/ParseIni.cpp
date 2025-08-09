#include "ParseIni.h"
#include "Logger.h"
#include <stdlib.h>

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
void ini_print_error_message(uint8_t e, bool eol = true)
{
    if (Serial)
    {
        switch (e)
        {
        case IniFile::errorNoError:
            logger::print("no error");
            break;
        case IniFile::errorFileNotFound:
            logger::print("file not found");
            break;
        case IniFile::errorFileNotOpen:
            logger::print("file not open");
            break;
        case IniFile::errorBufferTooSmall:
            logger::print("buffer too small");
            break;
        case IniFile::errorSeekError:
            logger::print("seek error");
            break;
        case IniFile::errorSectionNotFound:
            logger::print("section not found");
            break;
        case IniFile::errorKeyNotFound:
            logger::print("key not found");
            break;
        case IniFile::errorEndOfFile:
            logger::print("end of file");
            break;
        case IniFile::errorUnknownError:
            logger::print("unknown error");
            break;
        default:
            logger::print("unknown error value");
            break;
        }
        if (eol)
        {
            logger::print("\n");
        }
    }
}

void ini_parser(uint8_t *config_to_send)
{
    ini_parser("/config.ini", config_to_send);
}

void ini_parser(char *filename, uint8_t *config_to_send)
{
    ConfigData data; // ParseIni.h에서 작성했던, ConfigData

    // Set pin to select the SD card
    pinMode(SD_SELECT, OUTPUT); // SD_SELECT pin을 OUTPUT모드로 설정
    digitalWrite(SD_SELECT, HIGH);

    // Create buffer to hold the data read from the file
    const size_t buffer_len = ini_config::buffer_length;
    char buffer[buffer_len];

    // Setup the SPI to read the SD card
    SPI.begin();

    if (!SD.begin(SD_SELECT)) //  SD 카드 라이브러리를 시작하고, 실제로 SD 카드가 꽂혀 있고 정상적으로 인식되는지 확인
    {
        while (1) //  만약 SD 카드 인식에 실패하면, 프로그램이 더 이상 진행되지 않도록 무한 루프에 빠뜨려 멈추게 합니다.

            if (Serial)
            {
                logger::print("SD.begin() failed");
                logger::print("\n");
            }
    }

    IniFile ini(filename); //  IniFile 객체를 생성하고, 파일 이름을 지정하여 ini 파일을 열 준비를 합니다.

    if (!ini.open())
    {
        if (Serial)
        {
            logger::print("Ini file ");
            logger::print(filename);
            logger::print(" does not exist");
            logger::print("\n");
        }

        // Cannot do anything else
        while (1)
            ;
    }

    if (Serial) //  만약 Serial 통신이 가능하다면, 즉 Serial.begin()이 성공적으로 실행되었다면
    {
        logger::print("Ini file exists");
        logger::print("\n");
    }

    // Check the file is valid. This can be used to warn if any lines are longer than the buffer.
    if (!ini.validate(buffer, buffer_len))
    {
        if (Serial)
        {
            logger::print("ini file ");
            logger::print(ini.getFilename());
            logger::print(" not valid: ");
            ini_print_error_message(ini.getError());
        }

        // Cannot do anything else
        while (1)
            ;
    }

    get_section_key(ini, "Board", "name", buffer, buffer_len); // Read the key.
    data.board_name = buffer;                                  // Store the value

    config_to_send[config_defs::board_name_idx] = config_map::board_name[data.board_name]; // Encode the key to an uint8_t

    //--------------------------------------------------------

    get_section_key(ini, "Board", "version", buffer, buffer_len);
    data.board_version = buffer;

    config_to_send[config_defs::board_version_idx] = config_map::board_version[data.board_version];
    //--------------------------------------------------------

    get_section_key(ini, "Battery", "name", buffer, buffer_len);
    data.battery = buffer;

    config_to_send[config_defs::battery_idx] = config_map::battery[data.battery];
    //--------------------------------------------------------

    get_section_key(ini, "Exo", "name", buffer, buffer_len);
    data.exo_name = buffer;

    config_to_send[config_defs::exo_name_idx] = config_map::exo_name[data.exo_name];
    //--------------------------------------------------------

    // const char* 타입으로 변환하는 작업
    const char temp_exo_name[data.exo_name.length() + 1]; // 문자열의 끝을 나타내는 **널 문자(\0)**를 위해 길이에 1을 더합니다
    strcpy(temp_exo_name, data.exo_name.c_str());

    // Check the section that corresponds to the exo_name to get the correct parameters.
    get_section_key(ini, temp_exo_name, "sides", buffer, buffer_len);
    data.exo_sides = buffer;

    config_to_send[config_defs::exo_side_idx] = config_map::exo_side[data.exo_sides];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "knee", buffer, buffer_len);
    data.exo_knee = buffer;

    config_to_send[config_defs::knee_idx] = config_map::motor[data.exo_knee];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "ankle", buffer, buffer_len);
    data.exo_ankle = buffer;

    config_to_send[config_defs::ankle_idx] = config_map::motor[data.exo_ankle];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "kneeGearRatio", buffer, buffer_len);
    data.knee_gearing = buffer;

    config_to_send[config_defs::knee_gear_idx] = config_map::gearing[data.knee_gearing];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "ankleGearRatio", buffer, buffer_len);
    data.ankle_gearing = buffer;

    config_to_send[config_defs::ankle_gear_idx] = config_map::gearing[data.ankle_gearing];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "kneeDefaultController", buffer, buffer_len);
    data.exo_knee_default_controller = buffer;

    config_to_send[config_defs::exo_knee_default_controller_idx] = config_map::knee_controllers[data.exo_knee_default_controller];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "ankleDefaultController", buffer, buffer_len);
    data.exo_ankle_default_controller = buffer;

    config_to_send[config_defs::exo_ankle_default_controller_idx] = config_map::ankle_controllers[data.exo_ankle_default_controller];
    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "kneeUseLoadcell", buffer, buffer_len);
    data.knee_use_Loadcell = buffer;

    config_to_send[config_defs::knee_use_Loadcell_idx] = config_map::use_Loadcell[data.knee_use_Loadcell];

    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "ankleUseLoadcell", buffer, buffer_len);
    data.ankle_use_Loadcell = buffer;

    config_to_send[config_defs::ankle_use_Loadcell_idx] = config_map::use_Loadcell[data.ankle_use_Loadcell];

    //--------------------------------------------------------
    get_section_key(ini, temp_exo_name, "kneeUseIMU", buffer, buffer_len);
    data.knee_use_IMU = buffer;

    config_to_send[config_defs::knee_use_IMU_idx] = config_map::use_IMU[data.knee_use_IMU];

    //--------------------------------------------------------

    get_section_key(ini, temp_exo_name, "ankleUseIMU", buffer, buffer_len);
    data.ankle_use_IMU = buffer;

    config_to_send[config_defs::ankle_use_IMU_idx] = config_map::use_IMU[data.ankle_use_IMU];

    ini.close();
}

void get_section_key(IniFile ini, const char *section, const char *key, char *buffer, size_t buffer_len)
{
    //  getValue 함수를 호출하여 config.ini의 [section]에서 key에 해당하는 값을 읽어 buffer에 저장합니다. 이 함수가 성공적으로 값을 찾으면 true를 반환
    if (ini.getValue(section, key, buffer, buffer_len))
    {
        if (Serial)
        {
            // logger::print("section '");
            // logger::print(section);
            // logger::print("' has an entry '");
            // logger::print(key);
            // logger::print("' with value ");
            // logger::print(buffer);
            // logger::print("\n");
        }
    }
    else
    {
        if (Serial)
        {
            logger::print("Could not read '");
            logger::print(key);
            logger::print("' from section '");
            logger::print(section);
            logger::print("' , error was ");
            ini_print_error_message(ini.getError());
        }
    }
}

#endif
