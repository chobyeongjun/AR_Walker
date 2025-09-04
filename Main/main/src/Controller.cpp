#include "Controller.h"
#include "Logger.h"
#define CONTROLLER_DEBUG // Uncomment to enable debug statements to be printed to the serial monitor

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#include <math.h>
#include <random>
#include <cmath>
#include <Servo.h>

Servo myservo; // TO DO: Move servo definition code out of Controller.cpp

_Controller::_Controller(config_defs::joint_id id, ExoData *exo_data)
{
    _id = id;
    _data = exo_data;

    _t_helper = Time_Helper::get_instance();
    _t_helper_context = _t_helper->generate_new_context();
    _t_helper_delta_t = 0;

    // We just need to know the side to point at the right data location so it is only for the constructor
    bool is_left = utils::get_is_left(_id);

#ifdef CONTROLLER_DEBUG
    logger::print(is_left ? "Left " : "Right ");
#endif

    // Set _controller_data to point to the data specific to the controller.
    switch (utils::get_joint_type(_id))
    {
    case (uint8_t)config_defs::joint_id::knee:
#ifdef CONTROLLER_DEBUG
        logger::print("KNEE ");
#endif
        if (is_left)
        {
            _controller_data = &(exo_data->left_side.knee.controller);
            _joint_data = &(exo_data->left_side.knee);
        }
        else
        {
            _controller_data = &(exo_data->right_side.knee.controller);
            _joint_data = &(exo_data->right_side.knee);
        }
        break;

    case (uint8_t)config_defs::joint_id::ankle:
#ifdef CONTROLLER_DEBUG
        logger::print("ANKLE ");
#endif
        if (is_left)
        {
            _controller_data = &(exo_data->left_side.ankle.controller);
            _joint_data = &(exo_data->left_side.ankle);
        }
        else
        {
            _controller_data = &(exo_data->right_side.ankle.controller);
            _joint_data = &(exo_data->right_side.ankle);
        }
        break;
    }

#ifdef CONTROLLER_DEBUG
    logger::print("Controller : \n\t_controller_data set \n\t_joint_data set");
#endif

    // Added a pointer to the side data as most controllers will need to access info specific to their side.
    if (is_left)
    {
        _side_data = &(exo_data->left_side);
    }
    else
    {
        _side_data = &(exo_data->right_side);
    }

#ifdef CONTROLLER_DEBUG
    logger::println("\n\t_side_data set");
#endif

    // Controller initialization complete - using Admittance Control only
}


Step::Step(config_defs::joint_id id, ExoData *exo_data)
    : _Controller(id, exo_data)
{
#ifdef CONTROLLER_DEBUG
    Serial.println("Step::Step - Motor Test Controller Initialized");
#endif

    _start_time = millis();
    _current_step = 0;          // Start with initial delay
    _target_position = 0.0f;    // Start at 0 degrees
}

float Step::calc_motor_cmd()
{
    _update_step_sequence();
    return _target_position;
}

void Step::_update_step_sequence()
{
    unsigned long current_time = millis();
    unsigned long elapsed_time = current_time - _start_time;
    
    switch (_current_step) {
        case 0: // Initial delay (10 seconds)
            _target_position = 0.0f;
            if (elapsed_time > INITIAL_DELAY) {
                _current_step = 1;
                _start_time = current_time; // Reset timer
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Starting first movement to 90°");
#endif
            }
            break;
            
        case 1: // First movement to 90°
            _target_position = TEST_POSITION;
            if (elapsed_time > MOVE_DURATION) {
                _current_step = 2;
                _start_time = current_time;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Holding at 90°");
#endif
            }
            break;
            
        case 2: // Hold at 90°
            _target_position = TEST_POSITION;
            if (elapsed_time > HOLD_DURATION) {
                _current_step = 3;
                _start_time = current_time;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Returning to 0°");
#endif
            }
            break;
            
        case 3: // Return to 0°
            _target_position = 0.0f;
            if (elapsed_time > MOVE_DURATION) {
                _current_step = 4;
                _start_time = current_time;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Waiting before second cycle");
#endif
            }
            break;
            
        case 4: // Wait between cycles (2 seconds)
            _target_position = 0.0f;
            if (elapsed_time > 2000) {
                _current_step = 5;
                _start_time = current_time;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Starting second movement to 90°");
#endif
            }
            break;
            
        case 5: // Second movement to 90°
            _target_position = TEST_POSITION;
            if (elapsed_time > MOVE_DURATION) {
                _current_step = 6;
                _start_time = current_time;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Holding at 90° (second time)");
#endif
            }
            break;
            
        case 6: // Hold at 90° (second time)
            _target_position = TEST_POSITION;
            if (elapsed_time > HOLD_DURATION) {
                _current_step = 7;
                _start_time = current_time;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Final return to 0°");
#endif
            }
            break;
            
        case 7: // Final return to 0°
            _target_position = 0.0f;
            if (elapsed_time > MOVE_DURATION) {
                _current_step = 8;
#ifdef CONTROLLER_DEBUG
                Serial.println("Step Test: Sequence completed!");
#endif
            }
            break;
            
        case 8: // Test completed
            _target_position = 0.0f;
            // Stay at 0 degrees - test is done
            break;
    }
}

//****************************************************
// AdmittanceController Implementation
//****************************************************

AdmittanceController::AdmittanceController(config_defs::joint_id id, ExoData *exo_data)
    : _Controller(id, exo_data)
{
#ifdef CONTROLLER_DEBUG
    Serial.println("AdmittanceController::AdmittanceController");
#endif

    // Initialize default admittance parameters
    _mass = 1.0f;           // 1 kg virtual mass
    _damping = 50.0f;       // 50 Ns/m virtual damping
    _stiffness = 100.0f;    // 100 N/m virtual stiffness
    
    // Initialize gait phase-specific parameters
    // Swing phase: 더 유연한 파라미터 (보조 제공 시)
    _swing_mass = 0.5f;         // 더 작은 질량 (빠른 응답)
    _swing_damping = 30.0f;     // 적은 댐핑 (자유로운 움직임)
    _swing_stiffness = 50.0f;   // 적은 강성 (유연성)

    // Initialize force values
    _reference_force = 0.0f;    // Will be set from GUI or hardcoded
    _actual_force = 0.0f;
    
    // Initialize kinematic states
    _reference_position = 0.0f;
    _reference_velocity = 0.0f;
    _prev_reference_position = 0.0f;
    _prev_reference_velocity = 0.0f;
    
    // Simplified control: No complex modes needed
    
    // Initialize time tracking
    _prev_time = millis() / 1000.0f;
}

float AdmittanceController::calc_motor_cmd()
{
    // Get current time and calculate dt
    float current_time = millis() / 1000.0f;
    float dt = current_time - _prev_time;
    
    // Ensure reasonable dt (avoid division by zero or very small dt)
    if (dt <= 0.001f) {
        dt = 0.001f;  // 1ms minimum
    }
    if (dt > 0.1f) {
        dt = 0.1f;    // 100ms maximum to prevent instability
    }
    
    // Update control parameters based on current gait phase
    _update_control_parameters_based_on_gait_phase();
    
    // Calculate force error
    float force_error = _calculate_force_error();
    
    // Update reference kinematics using admittance control
    _update_reference_kinematics(force_error, dt);
    
    // Update time tracking
    _prev_time = current_time;
    
    // Return reference position as motor command (in degrees)
    return _reference_position;
}

float AdmittanceController::_calculate_force_error()
{
    // Get actual force from loadcell
    _actual_force = _get_actual_force_from_loadcell();
    
    // Get base reference force from ExoData (can be set from GUI)
    float base_reference_force = _data->get_reference_force(_id);
    
    // Simple logic: Swing uses reference force, Stance uses 0 (handled by Joint current control)
    if (_side_data->is_swing_phase()) {
        _reference_force = base_reference_force;
    } else {
        _reference_force = 0.0f;  // Stance: no admittance force (uses direct current)
    }
    
    // Calculate force error
    float force_error = _reference_force - _actual_force;
    
#ifdef CONTROLLER_DEBUG
    static unsigned long last_debug_time = 0;
    if (millis() - last_debug_time > 1000) {  // Debug every 1 second
        Serial.print("Admittance - Ref Force: ");
        Serial.print(_reference_force);
        Serial.print("N, Actual Force: ");
        Serial.print(_actual_force);
        Serial.print("N, Force Error: ");
        Serial.print(force_error);
        Serial.println("N");
        last_debug_time = millis();
    }
#endif
    
    return force_error;
}

void AdmittanceController::_update_reference_kinematics(float force_error, float dt)
{
    // Get current motor position as starting point
    float current_motor_position = 0.0f;
    if (_joint_data != nullptr) {
        current_motor_position = _joint_data->position;  // Current actual position
    }
    
    // Admittance control equation: M*a + D*v + K*x = F_error
    // Solve for acceleration: a = (F_error - D*v - K*x) / M
    
    float displacement = _reference_position - _prev_reference_position;
    float reference_acceleration = (force_error - _damping * _reference_velocity - _stiffness * displacement) / _mass;
    
    // Integrate to get velocity
    float new_velocity = _reference_velocity + reference_acceleration * dt;
    
    // Apply velocity limits for safety
    const float MAX_VELOCITY = 8000.0f;  // 8000 deg/s maximum
    new_velocity = constrain(new_velocity, -MAX_VELOCITY, MAX_VELOCITY);
    
    // Integrate to get relative displacement
    float relative_displacement = new_velocity * dt;
    
    // Apply relative displacement to current motor position
    float new_position = current_motor_position + relative_displacement;
    
    // Apply position limits for safety (relative to current position)
    const float MAX_DISPLACEMENT = 4500.0f;   // Maximum 4500degrees from current position
    const float MIN_DISPLACEMENT = -4500.0f;  // Minimum -4500 degrees from current position
    new_position = constrain(new_position, current_motor_position + MIN_DISPLACEMENT, current_motor_position + MAX_DISPLACEMENT);
    
    // Update states
    _prev_reference_position = _reference_position;
    _prev_reference_velocity = _reference_velocity;
    _reference_position = new_position;
    _reference_velocity = new_velocity;
}

float AdmittanceController::_get_actual_force_from_loadcell()
{
    // Get force reading from the joint's loadcell
    float force = 0.0f;
    
    if (_joint_data != nullptr) {
        force = _joint_data->loadcell_reading;  // Proper loadcell reading
    }
    
    return force;
}

void AdmittanceController::_update_control_parameters_based_on_gait_phase()
{
    // Only update parameters for swing phase (stance uses direct current control)
    if (_side_data->is_swing_phase()) {
        // Swing phase: 유연한 보조
        _mass = _swing_mass;
        _damping = _swing_damping;
        _stiffness = _swing_stiffness;
        
#ifdef CONTROLLER_DEBUG
        static unsigned long last_param_debug_time = 0;
        if (millis() - last_param_debug_time > 2000) {  // Debug every 2 seconds
            Serial.print("Admittance Params - SWING Phase");
            Serial.print(", M: ");
            Serial.print(_mass);
            Serial.print(", D: ");
            Serial.print(_damping);
            Serial.print(", K: ");
            Serial.println(_stiffness);
            last_param_debug_time = millis();
        }
#endif
    }
    // Note: Stance phase doesn't use admittance parameters (uses direct current control)
}


#endif