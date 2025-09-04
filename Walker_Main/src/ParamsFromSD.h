#ifndef ParamsFromSD_h
#define ParamsFromSD_h

#include "ExoData.h"
#include "ParseIni.h"
#include "Utilities.h"

#include <SD.h>
#include <SPI.h>
#include <map>
#include <string>

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#ifndef SD_SELECT
#define SD_SELECT BUILTIN_SDCARD
#endif

typedef std::map<uint8_t, std::string> ParamFilenameKey;


namespace param_error
{
    const uint8_t num_joint_ids = 3;                         /**< Number of bits the joint type ids need */
    const uint8_t SD_not_found_idx = num_joint_ids;          /**< Error when SD card isn't present */
    const uint8_t file_not_found_idx = SD_not_found_idx + 1; /**< Error when file is not found on the SD card */
}


namespace controller_parameter_filenames
{


    const ParamFilenameKey knee{
        
        {(uint8_t)config_defs::knee_controllers::step, "kneeControllers/step.csv"},
    };

    const ParamFilenameKey ankle{
        
        {(uint8_t)config_defs::ankle_controllers::step, "ankleControllers/step.csv"},
        
    };

};

void print_param_error_message(uint8_t error_type);


uint8_t set_controller_params(uint8_t joint_id, uint8_t controller_id, uint8_t set_num, ExoData *exo_data);

#endif
#endif