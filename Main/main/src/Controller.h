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

        // Simplified - only basic controller functionality needed
};


class Step : public _Controller
{
public:
    Step(config_defs::joint_id id, ExoData* exo_data);
    ~Step() {};

    float calc_motor_cmd();         /* Function that calculates the motor command. */

private:
    // Test sequence parameters
    unsigned long _start_time;      /* Time when controller started (millis) */
    int _current_step;              /* Current step in test sequence (0-4) */
    float _target_position;         /* Current target position */
    
    // Test configuration
    static const unsigned long INITIAL_DELAY = 10000;    // 10 seconds initial delay
    static const unsigned long MOVE_DURATION = 3000;     // 3 seconds to reach position
    static const unsigned long HOLD_DURATION = 2000;     // 2 seconds hold
    static const float TEST_POSITION = 90.0f;            // 90 degrees test position
    
    // Step sequence: 0=wait, 1=move1, 2=hold1, 3=return1, 4=wait2, 5=move2, 6=hold2, 7=return2, 8=done
    void _update_step_sequence();
};

class AdmittanceController : public _Controller
{
public:
    AdmittanceController(config_defs::joint_id id, ExoData* exo_data);
    ~AdmittanceController() {};

    float calc_motor_cmd();         /* Calculates reference position based on admittance control */

private:
    // Admittance control parameters (상태별로 다르게 적용)
    float _mass;                    /* Virtual mass (kg) */
    float _damping;                 /* Virtual damping (Ns/m) */
    float _stiffness;               /* Virtual stiffness (N/m) */
    
    // 보행 상태별 파라미터
    float _swing_mass, _swing_damping, _swing_stiffness;       /* Swing phase parameters */
    
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
    
    // Simplified control: Only swing admittance + stance current
    
    // Internal calculation methods
    float _calculate_force_error();                                    /* Force error calculation */
    void _update_reference_kinematics(float force_error, float dt);    /* Update position/velocity */
    float _get_actual_force_from_loadcell();                          /* Get force from loadcell */
    void _update_control_parameters_based_on_gait_phase();             /* Update parameters based on gait phase */
    
    // Admittance control equation: M*a + D*v + K*x = F_error
    // Solved for acceleration: a = (F_error - D*v - K*x) / M
};



#endif
#endif