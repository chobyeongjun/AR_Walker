#ifndef Controllerdata_h
#define ControllerData_h
#include <stdint.h>

#include "Arduino.h"

#include "Board.h"
#include "ParseIni.h"
#include <stdint.h>

//Forward declaration
class ExoData;

namespace controller_defs                   /**< Simplified for Admittance Control only */
{ 
    namespace step                                              //Basic step parameters
    {
        const uint8_t amplitude_idx = 0;                        //Magnitude of the applied torque in Nm             
        const uint8_t duration_idx = 1;                         //Duration of the applied torque
        const uint8_t num_parameter = 2;                        //Reduced parameter count
    }
}

class ControllerData {
	public:
        ControllerData(config_defs::joint_id id, uint8_t* config_to_send);
        

        void reconfigure(uint8_t* config_to_send);
        
        uint8_t get_parameter_length();
        
        
        uint8_t controller;                                 /**< Id of the current controller */
        config_defs::JointType joint;                       /**< Id of the current joint */

        float setpoint;                                     /**< Controller setpoint (motor command) */
        float parameters[3];                                /**< Basic parameters for Step and Admittance controllers */
        uint8_t parameter_set;                              /**< Parameter set index */

        float filtered_torque_reading;                      /**< Filtered torque reading */
        float filtered_cmd;                                 /**< Filtered motor command */
        
        // Simplified - removed all unused controller-specific variables
};      

#endif
