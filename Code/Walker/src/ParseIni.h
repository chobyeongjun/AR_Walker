#ifndef ParseIni_h
#define ParseIni_h

#include <stdint.h>

namespace ini_config
{
    const int buffer_length = 500;
    const int key_length = 25;
    const int section_length = 10;
    const int number_of_keys = 25; // 키 개수에 맞게 수정
}

namespace config_defs
{
    enum class board_name : uint8_t
    {
        AK_board = 1,
    };

    enum class board_version : uint8_t
    {
        zero_five_one = 1,
    };

    enum class battery : uint8_t
    {
        smart = 1,
        dumb = 2,
    };

    enum class exo_name : uint8_t
    {
        bilateral_ankle = 1,
        bilateral_knee = 2,
        left_ankle = 3,
        right_ankle = 4,
        left_knee = 5,
        right_knee = 6,
        test = 7,
    };

    enum class exo_side : uint8_t
    {
        bilateral = 1,
        left = 2,
        right = 3,
    };

    enum class JointType
    {
        knee = 1,
        ankle = 2,
    };

    enum class motor : uint8_t
    {
        not_used = 1,
        AK60 = 2,
        AK80 = 3,
        AK60_v1_1 = 4,
        AK70 = 5,
        MaxonMotor = 6,
    };

    enum class gearing : uint8_t
    {
        gearing_1_1 = 1,
        gearing_2_1 = 2,
        gearing_3_1 = 3,
        gearing_4_5_1 = 4,
    };

    enum class joint_id : uint8_t
    {
        left = 0b01000000,
        right = 0b00100000,

        knee = 0b00000001,
        ankle = 0b00000010,

        left_knee = left | knee,
        left_ankle = left | ankle,
        right_knee = right | knee,
        right_ankle = right | ankle,
    };

    enum class knee_controllers : uint8_t
    {
        disabled = 1,
        zero_torque = 2,
        constant_torque = 3,
        chirp = 4,
        step = 5,
    };

    enum class ankle_controllers : uint8_t
    {
        disabled = 1,
        zero_torque = 2,
        pjmc = 3,
        zhang_collins = 4,
        constant_torque = 5,
        trec = 6,
        calibr_manager = 7,
        chirp = 8,
        step = 9,
        spv2 = 10,
        pjmc_plus = 11,
    };

    enum class use_Loadcell : uint8_t
    {
        no = 1,
        yes = 2,
    };

    enum class flip_angle_dir : uint8_t
    {
        neither = 1,
        left = 2,
        right = 3,
        both = 4,
    };

    static const int board_name_idx = 0;
    static const int board_version_idx = 1;
    static const int battery_idx = 2;
    static const int exo_name_idx = 3;
    static const int exo_side_idx = 4;
    static const int knee_idx = 5;
    static const int ankle_idx = 6;
    static const int knee_gear_idx = 7;
    static const int ankle_gear_idx = 8;
    static const int exo_knee_default_controller_idx = 9;
    static const int exo_ankle_default_controller_idx = 10;
    static const int knee_use_Loadcell_idx = 11;
    static const int ankle_use_Loadcell_idx = 12;
    static const int left_knee_Loadcell_ref_weight_idx = 13;
    static const int right_knee_Loadcell_ref_weight_idx = 14;
    static const int left_ankle_Loadcell_ref_weight_idx = 15;
    static const int right_ankle_Loadcell_ref_weight_idx = 16;
    static const int left_knee_LoadcellZeroOffset_idx = 17;
    static const int right_knee_LoadcellZeroOffset_idx = 18;
    static const int left_ankle_LoadcellZeroOffset_idx = 19;
    static const int right_ankle_LoadcellZeroOffset_idx = 20;
    static const int left_knee_LoadcellSensitivity_idx = 21;
    static const int right_knee_LoadcellSensitivity_idx = 22;
    static const int left_ankle_LoadcellSensitivity_idx = 23;
    static const int right_ankle_LoadcellSensitivity_idx = 24;
    static const int left_knee_IMU_ID_idx = 25;
    static const int right_knee_IMU_ID_idx = 26;
    static const int left_ankle_IMU_ID_idx = 27;
    static const int right_ankle_IMU_ID_idx = 28;
}

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include <SD.h>
#include <SPI.h>
#include "IniFile.h"

#include <map>
#include <string>

#ifndef SD_SELECT
#define SD_SELECT BUILTIN_SDCARD
#endif

void ini_parser(uint8_t *config_to_send);

void ini_parser(char *filename, uint8_t *config_to_send);

void get_section_key(IniFile ini, const char *section, const char *key, char *buffer, size_t buffer_len);

void ini_print_error_message(uint8_t e, bool eol = true);

namespace config_map
{
    typedef std::map<std::string, uint8_t> IniKeyCode;

    const IniKeyCode board_name =
        {
            {"AK_Board", (uint8_t)config_defs::board_name::AK_board},
    };

    const IniKeyCode board_version =
        {
            {"0.5.1", (uint8_t)config_defs::board_version::zero_five_one},
    };

    const IniKeyCode battery =
        {
            {"smart", (uint8_t)config_defs::battery::smart},
            {"dumb", (uint8_t)config_defs::battery::dumb},
    };

    const IniKeyCode exo_name{
        {"bilateralAnkle", (uint8_t)config_defs::exo_name::bilateral_ankle},
        {"bilateralKnee", (uint8_t)config_defs::exo_name::bilateral_knee},
        {"leftAnkle", (uint8_t)config_defs::exo_name::left_ankle},
        {"rightAnkle", (uint8_t)config_defs::exo_name::right_ankle},
        {"leftKnee", (uint8_t)config_defs::exo_name::left_knee},
        {"rightKnee", (uint8_t)config_defs::exo_name::right_knee},
        {"test", (uint8_t)config_defs::exo_name::test},
    };

    const IniKeyCode exo_side{
        {"bilateral", (uint8_t)config_defs::exo_side::bilateral},
        {"left", (uint8_t)config_defs::exo_side::left},
        {"right", (uint8_t)config_defs::exo_side::right},
    };

    const IniKeyCode motor{
        {"0", (uint8_t)config_defs::motor::not_used},
        {"AK60", (uint8_t)config_defs::motor::AK60},
        {"AK80", (uint8_t)config_defs::motor::AK80},
        {"AK60v1.1", (uint8_t)config_defs::motor::AK60_v1_1},
        {"AK70", (uint8_t)config_defs::motor::AK70},
        {"MaxonMotor", (uint8_t)config_defs::motor::MaxonMotor},
    };

    const IniKeyCode gearing{
        {"1", (uint8_t)config_defs::gearing::gearing_1_1},
        {"2", (uint8_t)config_defs::gearing::gearing_2_1},
        {"3", (uint8_t)config_defs::gearing::gearing_3_1},
        {"4.5", (uint8_t)config_defs::gearing::gearing_4_5_1},
    };

    const IniKeyCode knee_controllers{
        {"0", (uint8_t)config_defs::knee_controllers::disabled},
        {"zeroTorque", (uint8_t)config_defs::knee_controllers::zero_torque},
        {"constantTorque", (uint8_t)config_defs::knee_controllers::constant_torque},
        {"chirp", (uint8_t)config_defs::knee_controllers::chirp},
        {"step", (uint8_t)config_defs::knee_controllers::step},
    };

    const IniKeyCode ankle_controllers{
        {"0", (uint8_t)config_defs::ankle_controllers::disabled},
        {"zeroTorque", (uint8_t)config_defs::ankle_controllers::zero_torque},
        {"PJMC", (uint8_t)config_defs::ankle_controllers::pjmc},
        {"zhangCollins", (uint8_t)config_defs::ankle_controllers::zhang_collins},
        {"constantTorque", (uint8_t)config_defs::ankle_controllers::constant_torque},
        {"TREC", (uint8_t)config_defs::ankle_controllers::trec},
        {"calibrManager", (uint8_t)config_defs::ankle_controllers::calibr_manager},
        {"chirp", (uint8_t)config_defs::ankle_controllers::chirp},
        {"step", (uint8_t)config_defs::ankle_controllers::step},
        {"SPV2", (uint8_t)config_defs::ankle_controllers::spv2},
        {"PJMC_PLUS", (uint8_t)config_defs::ankle_controllers::pjmc_plus},
    };

    const IniKeyCode use_Loadcell{
        {"0", (uint8_t)config_defs::use_Loadcell::no},
        {"yes", (uint8_t)config_defs::use_Loadcell::yes},
    };

    const IniKeyCode IMU_ID{
        {"16", 16}, // IMU 사용하지 않을 때
        {"0", 0},
        {"1", 1},
        {"2", 2},
        {"3", 3},
    };

};

struct ConfigData
{

    std::string board_name;
    std::string board_version;

    std::string battery;

    std::string exo_name;
    std::string exo_sides;

    std::string exo_knee;
    std::string exo_ankle;

    std::string knee_gearing;
    std::string ankle_gearing;

    std::string exo_knee_default_controller;
    std::string exo_ankle_default_controller;

    std::string knee_use_Loadcell;
    std::string ankle_use_Loadcell;

    float left_knee_Loadcell_ref_weight;
    float right_knee_Loadcell_ref_weight;
    float left_ankle_Loadcell_ref_weight;
    float right_ankle_Loadcell_ref_weight;

    float left_knee_LoadcellZeroOffset;
    float right_knee_LoadcellZeroOffset;
    float left_ankle_LoadcellZeroOffset;
    float right_ankle_LoadcellZeroOffset;

    float left_knee_LoadcellSensitivity;
    float right_knee_LoadcellSensitivity;
    float left_ankle_LoadcellSensitivity;
    float right_ankle_LoadcellSensitivity;

    std::string left_knee_IMU_ID;
    std::string right_knee_IMU_ID;
    std::string left_ankle_IMU_ID;
    std::string right_ankle_IMU_ID;
};
#endif
#endif