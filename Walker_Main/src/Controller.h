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
    int flag;
    float difference;
    float turn;
    float flag_time;
    float change_time;

    float calc_motor_cmd();         /* Function that calculates the motor command. */

};

class AdmittanceController : public _Controller
{
public:
    AdmittanceController(config_defs::joint_id id, ExoData* exo_data);
    ~AdmittanceController() {};

    float calc_motor_cmd();         /* Calculates reference position based on admittance control */

private:
    // Admittance control parameters
    float _mass;                    /* Virtual mass (kg) */
    float _damping;                 /* Virtual damping (Ns/m) */
    float _stiffness;               /* Virtual stiffness (N/m) */
    
    // Reference and actual force
    float _reference_force;         /* Desired force (N) */
    float _actual_force;            /* Measured force from loadcell (N) */
    
    // Position and velocity tracking
    float _reference_position;      /* Calculated reference position (degrees) */
    float _reference_velocity;      /* Calculated reference velocity (deg/s) */
    float _prev_reference_position; /* Previous reference position */
    float _prev_reference_velocity; /* Previous reference velocity */
    
    // Time tracking for integration
    float _prev_time;               /* Previous time for dt calculation */
    
    // Internal calculation methods
    float _calculate_force_error();                                    /* Force error calculation */
    void _update_reference_kinematics(float force_error, float dt);    /* Update position/velocity */
    float _get_actual_force_from_loadcell();                          /* Get force from loadcell */
    
    // Admittance control equation: M*a + D*v + K*x = F_error
    // Solved for acceleration: a = (F_error - D*v - K*x) / M
};

class PositionController : public _Controller
{
public:
    PositionController(config_defs::joint_id id, ExoData* exo_data);
    ~PositionController() {};

    float calc_motor_cmd();         /* Calculates position command for swing phase */

private:
    float _target_position;         /* Target position for swing (degrees) */
    float _current_position;        /* Current joint position (degrees) */
    
    // PID parameters for position control
    float _kp_pos;                  /* Position proportional gain */
    float _ki_pos;                  /* Position integral gain */
    float _kd_pos;                  /* Position derivative gain */
    
    // Swing trajectory parameters
    float _swing_start_position;    /* Position at swing start */
    float _swing_target_position;   /* Desired position at swing end */
    
    float _calculate_swing_trajectory();  /* Calculate desired position during swing */
};

class SpeedController : public _Controller
{
public:
    SpeedController(config_defs::joint_id id, ExoData* exo_data);
    ~SpeedController() {};

    float calc_motor_cmd();         /* Calculates speed command for swing phase */

private:
    float _target_speed;            /* Target speed for swing (deg/s) */
    float _current_speed;           /* Current joint speed (deg/s) */
    
    // PID parameters for speed control
    float _kp_speed;                /* Speed proportional gain */
    float _ki_speed;                /* Speed integral gain */
    float _kd_speed;                /* Speed derivative gain */
    
    // Swing speed profile parameters
    float _max_swing_speed;         /* Maximum speed during swing */
    float _acceleration_phase;      /* Acceleration phase percentage (0-1) */
    float _deceleration_phase;      /* Deceleration phase percentage (0-1) */
    
    float _calculate_swing_speed_profile();  /* Calculate desired speed during swing */
};


#endif
#endif