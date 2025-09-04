#ifndef Controllerdata_h
#define ControllerData_h
#include <stdint.h>

#include "Arduino.h"

#include "Board.h"
#include "ParseIni.h"
#include <stdint.h>

//Forward declaration
class ExoData;

namespace controller_defs                   /**< Stores the parameter indexes for different controllers */
{ 

    namespace step                                              //Parameters for step torque used in max torque capacity testing
    {
        const uint8_t amplitude_idx = 0;                        //Magnitude of the applied torque in Nm             
        const uint8_t duration_idx = 1;                         //Duration of the applied torque
        const uint8_t repetitions_idx = 2;                      //Number of times the torque is applied
        const uint8_t spacing_idx = 3;                          //Time between each application of torque
        const uint8_t pid_flag_idx = 4;                         //Flag to determine whether or not PID used
        const uint8_t p_gain_idx = 5;                           //Value of P Gain for PID control
        const uint8_t i_gain_idx = 6;                           //Value of I Gain for PID control
        const uint8_t d_gain_idx = 7;                           //Value of D Gain for PID control
        const uint8_t alpha_idx = 8;                            //Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise.
        const uint8_t num_parameter = 9;
    }

  
}

class ControllerData {
	public:
        ControllerData(config_defs::joint_id id, uint8_t* config_to_send);
        

        void reconfigure(uint8_t* config_to_send);
        
        uint8_t get_parameter_length();
        
        
        uint8_t controller;                                 /**< Id of the current controller */
        config_defs::JointType joint;                       /**< Id of the current joint */

        float reference_force;
        float setpoint;                                     /**< Controller setpoint, basically the motor command. */
        float ff_setpoint;                                  /**< Feed forwared setpoint, only updated in closed loop controllers */
        float parameters[10];  /**< Parameter list for the controller see the controller_defs namespace for the specific controller. */
        uint8_t parameter_set;                              /**< Temporary value used to store the parameter set while we are pulling from the sd card. */

        float filtered_torque_reading;                      /**< Filtered torque reading, used for filtering torque signal */
        float filtered_cmd;                                 /**< Filtered command, used for filtering motor commands */
        float filtered_setpoint;                            /**< Filtered setpoint for the controller */
        
        //Variables for Auto Kf in the PID Controller
        float kf = 1;                                       /**< Gain for the controller */
        float prev_max_measured = 0;                        /**< Previous max measured value */
        float prev_max_setpoint = 0;                        /**< Previous max setpoint value */
        float max_measured = 0;                             /**< Max measured value */
        float max_setpoint = 0;                             /**< Max setpoint value */

        /* Controller Specific Variables That You Want To Plot. If you do not want to plot, than put variables in Controller.h under the controller of interest. */

        //Variables for TREC Controller (MOVE TO Controller.h)
        float reference_angle = 0;                              /**< Reference angle for the spring term */
        float reference_angle_offset = 0;                       /**< Offset for the reference angle */
        bool reference_angle_updated = false;                   /**< Flag to indicate if the reference angle was updated this step */
        float filtered_squelched_supportive_term = 0;           /**< Low pass on final spring output */
        float neutral_angle = 0.0f;                             /**< Neutral angle for the spring term */
        bool prev_calibrate_trq_sensor = false;                 /**< Previous value of the calibrate torque sensor flag */
        const float cal_neutral_angle_alpha = 0.01f;            /**< Alpha for the low pass on the neutral angle calibration */
        float level_entrance_angle = 0.0f;                      /**< Level entrance angle for the spring term */
        bool prev_calibrate_level_entrance = false;             /**< Previous value of the calibrate level entrance flag */
        const float cal_level_entrance_angle_alpha = 0.01f;     /**< Alpha for the low pass on the level entrance calibration */
		float stateless_pjmc_term = 0;
		float toeFsrThreshold = 0.2f;
		bool wait4HiHeelFSR = false;
		uint16_t iPidHiTorque = 0;
		bool pausePid = false;
		float currentTime = 0.0000f;
		float previousTime = 0.0000f;
		float itrTime = 0.0000f;
		uint8_t numBelow500 = 0;
		int maxTorqueCache = 0;
		float previousMaxCmdCache = 15;
		float previousMinCmdCache = -15;
		float currentMaxCmdCache = 0;
		float currentMinCmdCache = 0;
		uint16_t cmdCacheItr = 0;
		bool doIncrUpperLmt = false;
		bool doIncrLowerLmt = false;
		float setpoint2use = 0;
		float maxPjmcSpringDamper = 0;
		bool wasStance = false;
		float prevMaxPjmcSpringDamper = 0;
		float cmd_2nd = 0;
		float cmd_1st = 0;	

        //Variables for the ElbowMinMax Controller
        float FlexSense;
        float ExtenseSense;
		
		//Variables for the Calibration Manger "Controller"
		bool calibrComplete = false;
		uint16_t iCalibr = 0;
		int PIDMLTPLR = 0;
		bool calibrStart = false;
		float calibrSum = 0;
		
		//Variables for the Zhang-Collins Controller
		float previous_cmd = 0;
		
        //Variables for SPV2
		int plotting_scalar = 1;                //Maxon servo interrupter
		unsigned long servo_departure_time;
		bool servo_did_go_down = true;
		bool servo_get_ready = false;

        //Variables for the PHMC Controller
        float fs;
        float state;
};      

#endif
