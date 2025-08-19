#ifndef Controller_h
#define Controller_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include <Arduino.h>

#include "ExoData.h"
#include "Board.h"
#include "ParseIni.h"
#include <stdint.h>
#include "Utilities.h"
#include "config.h"
#include "Time_Helper.h"
#include <algorithm>
#include <utility>


class _Controller
{
	public:
        
        _Controller(config_defs::joint_id id, ExoData* exo_data);
		
        
        virtual ~_Controller(){};
        

		virtual float calc_motor_cmd() = 0; 
        
        
        void reset_integral(); 
        
    protected:
        
        ExoData* _data;                     /**< Pointer to the full data instance*/
        ControllerData* _controller_data;   /**< Pointer to the data associated with this controller */
        SideData* _side_data;               /**< Pointer for the side data the controller is associated with */
        JointData* _joint_data;             /**< Pointer to the joint data the controller is associated with */
         
        config_defs::joint_id _id;          /**< Id of the joint this controller is attached to. */
        
        Time_Helper* _t_helper;             /**< Instance of the time helper to track when things happen used to check if we have a set time for the PID */
        float _t_helper_context;            /**< Store the context for the timer helper */
        float _t_helper_delta_t;            /**< Time time since the last event */

        //Values for the PID controller
        float _pid_error_sum = 0;           /**< Summed error term for calucating intergral term */
        float _prev_input;                  /**< Prev error term for calculating derivative */
        float _prev_de_dt;                  /**< Prev error derivative used if the timestep is not good*/
        float _prev_pid_time;               /**< Prev time the PID was called */
        
        //Parameters for Servo Control (SPV2)
		long pos;
		unsigned long servoWatch;
		int pos1;
		int pos2;
		bool _do_stop_servo = false;

        float _pid(float cmd, float measurement, float p_gain, float i_gain, float d_gain);
		

		int _servo_runner(uint8_t servo_pin, uint8_t speed_level, long angle_initial, long angle_final);
		

        float _pjmc_generic(float current_fsr, float fsr_threshold, float setpoint_positive, float setpoint_negative);
        
        //Values for the Compact Form Model Free Adaptive Controller
        std::pair<float, float> measurements;
        std::pair<float, float> outputs;
        std::pair<float, float> phi;            /**< Psuedo partial derivative */
        float rho;                              /**< Penalty factor (0,1) */
        float lamda;                            /**< Weighting factor limits delta u */
        float etta;                             /**< Step size constant (0, 1] */
        float mu;                               /**< Weighting factor that limits the variance of u */
        float upsilon;                          /**< A sufficiently small integer ~10^-5 */
        float phi_1;                            /**< Initial/reset condition for estimation of psuedo partial derivitave */
        
        float _cf_mfac(float reference, float current_measurement);
};


class TREC : public _Controller
{
public:
    TREC(config_defs::joint_id id, ExoData* exo_data);
    ~TREC() {};

    float calc_motor_cmd();

private:
    void _update_reference_angles(SideData* side_data, ControllerData* controller_data, float percent_grf, float percent_grf_heel);
    void _capture_neutral_angle(SideData* side_data, ControllerData* controller_data);
    void _grf_threshold_dynamic_tuner(SideData* side_data, ControllerData* controller_data, float threshold, float percent_grf_heel);
    void _plantar_setpoint_adjuster(SideData* side_data, ControllerData* controller_data, float pjmcSpringDamper);
};


class ProportionalJointMoment : public _Controller
{
    public:
        ProportionalJointMoment(config_defs::joint_id id, ExoData* exo_data);
        ~ProportionalJointMoment(){};
        
        float calc_motor_cmd();
    private:
        std::pair<float, float> _stance_thresholds_left, _stance_thresholds_right;
        
        float _inclination_scaling{1.0f};
};


class ZeroTorque : public _Controller
{
    public:
        ZeroTorque(config_defs::joint_id id, ExoData* exo_data);
        ~ZeroTorque(){};
        
        float calc_motor_cmd();
};


class ZhangCollins: public _Controller
{
    public:
        ZhangCollins(config_defs::joint_id id, ExoData* exo_data);
        ~ZhangCollins(){};
        
        float calc_motor_cmd();

        float _spline_generation(float node1, float node2, float node3, float torque_magnitude, float percent_gait);

        float torque_cmd;
		float cmd;
};


class FranksCollinsHip: public _Controller
{
    public:
        FranksCollinsHip(config_defs::joint_id id, ExoData* exo_data);
        ~FranksCollinsHip(){};
       
        float calc_motor_cmd();

        float _spline_generation(float node1, float node2, float node3, float torque_magnitude, float shifted_percent_gait);

        float last_percent_gait;
        float last_start_time;
       
};


class ConstantTorque : public _Controller
{
public:
    ConstantTorque(config_defs::joint_id id, ExoData* exo_data);
    ~ConstantTorque() {};

    float calc_motor_cmd();

    float previous_command;         /* Stores Previous Loop's Torque Command */
    float previous_torque_reading;  /* Stores Previous Loop's Measured Torque */
    int flag;                       /* Flag that Determines Filter Status */
    float difference;               /* Stores Difference in Command when Changed */

};


class ElbowMinMax : public _Controller
{
public:
    ElbowMinMax(config_defs::joint_id id, ExoData* exo_data);
    ~ElbowMinMax() {};

    float alpha0;
    float alpha1;
    float alpha2;
    float alpha3;

    float cmd;

    float Smoothed_Sig_Flex;
    float Smoothed_Sig_Ext;
    float Smoothed_Flex_Max;
    float Smoothed_Flex_Min;
    float Smoothed_Ext_Max;
    float Smoothed_Ext_Min;

    float starttime;

    float check;

    float Angle_Max;
    float Angle_Min;
    float Angle;

    bool flexState;
    bool extState;
    bool nullState;

    float previous_setpoint;

    float fsr_toe_previous_elbow;
    float fsr_heel_previous_elbow;

    float SpringEffect;

    float calc_motor_cmd();
    
};

class CalibrManager : public _Controller
{
public:
    CalibrManager(config_defs::joint_id id, ExoData* exo_data);
    ~CalibrManager() {};

    float calc_motor_cmd();
};


class Chirp : public _Controller
{
public:
    Chirp(config_defs::joint_id id, ExoData* exo_data);
    ~Chirp() {};

    float start_flag;               /* Flag that triggers recording of the initial start time of the controller upon usage. */
    float start_time;               /* Variable that stores the start time of the controller. */
    float current_time;             /* Variable that stores the current time of the controller. */
    float previous_amplitude;       /* Variable that stores the previous amplitude, used as a switch to restart the controller if needed. (Set amplitude to 0 and then set to desired amplitude). */

    float calc_motor_cmd();         /* Function that calculates the motor command. */

};

class Step : public _Controller
{
public:
    Step(config_defs::joint_id id, ExoData* exo_data);
    ~Step() {};

    int n;                          /* Keeps track of how many steps have been performed. */
    int start_flag;                 /* Flag that triggers the recording of the time that the step is first applied. */
    float start_time;               /* Time that the step was first applied. */
    float cmd_ff;                   /* Motor command. */
    float previous_time;            /* Stores time from previous iteration. */
    float end_time;                 /* Records time that step ended. */

    float previous_command;
    float previous_torque_reading;
    int flag;
    float difference;
    float turn;
    float flag_time;
    float change_time;

    float calc_motor_cmd();         /* Function that calculates the motor command. */

};

class ProportionalHipMoment : public _Controller
{
public:
    ProportionalHipMoment(config_defs::joint_id id, ExoData* exo_data);
    ~ProportionalHipMoment() {};
    
    /* Note: Duration in this controller is in terms of number of iterations in that window, rather than as a time. */

    int state;                      /* Keeps track of what state we are in: State 1 - Mid-to-Late Swing (15% onward), State 2: Stance, State 3: Early-Swing (First 15%). */

    bool first_state2;              /* Flag to set variable values upon the first instance of the current State 2. */
    bool first_state3;              /* Flag to set variable values upon the first instance of the current State 3. */

    int swing_counter;              /* Keeps track of the number of iterations that have occured in current swing phase. */
    int state1_counter;             /* Keeps track of the number of iterations that have occured in current State 1. */
    int prev_state1_counter;        /* Stores the previous duration of State 1, used to estimate position in current State 1 relative to expected duration. */
    int stance_counter;             /* Keeps track of the number of iterations that have occured in current Stance Phase. */
    int swing_duration;             /* Stores the duration of the previous swing phase. */
 
    float setpoint;                 /* Stores the calculated feed-foward setpoint for the hip command. */
    float old_setpoint;             /* Stores the setpoint at the end of State 3 to be used for setpoint calculation in State 1. */

    int state_count_12;             /* Keeps track of the number of iterations that have occured in the State 1 - to - State 2 Transition. */
    int state_count_23;             /* Keeps track of the number of iterations that have occured in the State 2 - to - State 3 Transition. */
    int state_count_31;             /* Keeps track of the number of iterations that have occured in the State 3 - to - State 1 Transition. */
    
    int Prev_latestance_duration;   /* Stores the previous duration of the late-stance phase (part of the late-stance to early-swing transition period. */
    int latestance_duration;        /* Records the duration of the recently ended late-stance phase. */
    int latestance_counter;         /* Keeps track of the number of iterations that have occured in the current Late-Stance Period. */
    float Alpha_counter;            /* Keeps track of the number of iterations that have occured in the current Late-Stance - and - Early Swing Transition Period. */
    float Alpha;                    /* Stores the expected duration of the Late-Stance - and - Early Swing Transition Period, based on the duration of the last transition period. */
    float t;                        /* Calculated percentage of stance-to-swing transition based on the duration of the previous stance-to-swing transition (expressed as 0.1, 0.2,... rather than 10%, 20%,...). */
    
    float fs;                       /* Ratio of heel and toe fsrs accounting for GRF Ratio (0.25). */
    float fs_min;                   /* Stores the minimum calculated fs for the current cycle, used as a starting estimate for the Late-Stance - to - Early Swing transition period. */
    float prev_fs;                  /* Stores the previous cycle's fs to help determine the slope of the fs curve. */
    float hip_ratio;                /* Part of calculation to determine the feed-foward setpoint calculation during stance-phase (State 2). */

    float calc_motor_cmd();         /* Function to calcualte the desired motor command. */

private:

};

class SPV2 : public _Controller
{
public:
    SPV2(config_defs::joint_id id, ExoData* exo_data);
    ~SPV2() {};

    float calc_motor_cmd();

private:

};


class PJMC_PLUS : public _Controller
{
public:
    PJMC_PLUS(config_defs::joint_id id, ExoData* exo_data);
    ~PJMC_PLUS() {};

    float calc_motor_cmd();

private:

};

#endif
#endif