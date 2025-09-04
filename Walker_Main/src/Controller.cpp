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

    _prev_input = 0;
    _prev_de_dt = 0;

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

    // Set the parameters for cf mfac
    measurements.first = -1;
    measurements.second = 1;
    outputs.first = 0;
    outputs.second = 0;
    phi.first = 2;
    phi.second = 0;
    rho = 0.5;
    lamda = 2;
    etta = 1;
    mu = 1;
    upsilon = 1 / (pow(10, 5));
    phi_1 = phi.first;
}

//****************************************************

float _Controller::_pid(float cmd, float measurement, float p_gain, float i_gain, float d_gain)
{
    // Check if time is ok
    bool time_good = true;

    if (_t_helper->tick(_t_helper_context) > ((float)1 / LOOP_FREQ_HZ * 1000000 * (1 + LOOP_TIME_TOLERANCE)))
    {
        time_good = false;
    }

    // Record the current time
    float now = micros();

    // Record the change in time
    float dt = (now - _prev_pid_time) * 1000000;

    // Calculate the difference in the prescribed and measured torque
    float error_val = cmd - measurement;

    // If we want to to include the integral term (Note: We generally do not like to use the I gain but we have it here for completeness)
    if (i_gain != 0)
    {
        _pid_error_sum += error_val / LOOP_FREQ_HZ;
    }
    else
    {
        _pid_error_sum = 0;
    }

    // Get the current status of the exskleton
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on);

    // Reset the integral term if the user pauses the trial or we are no longer in an active trial
    if (_data->user_paused || !active_trial)
    {
        _pid_error_sum = 0;
    }

    // Initialize the derivative of the error
    float de_dt = 0;

    // Calculate the derivative of the erro
    if (time_good)
    {
        de_dt = -(measurement - _prev_input) * (1000.0f / LOOP_FREQ_HZ); // Convert to ms
        _prev_de_dt = de_dt;
    }
    else
    {
        de_dt = 0;
    }

    // Set the previous times for the next loop through the controller
    _prev_pid_time = now;
    _prev_input = measurement;

    // Calculate the individual P,I,and D Terms
    float p = p_gain * error_val;
    float i = i_gain * _pid_error_sum;
    float d = d_gain * de_dt;

    // Return the summed PID
    return p + i + d;
}

//****************************************************

Step::Step(config_defs::joint_id id, ExoData *exo_data)
    : _Controller(id, exo_data)
{
#ifdef CONTROLLER_DEBUG
    Serial.println("Step::Step");
#endif

    // Initializes Values
    n = 1;
    start_flag = 1;
    start_time = 0;
    cmd_ff = 0;
    end_time = 0;

    previous_command = 0;
    flag = 0;
    difference = 0;
    turn = 0;
    flag_time = 0;
    change_time = 0;
}

float Step::calc_motor_cmd()
{

    return 0.0f;
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

    // Initialize admittance parameters with safe default values
    _mass = 1.0f;           // 1 kg virtual mass
    _damping = 50.0f;       // 50 Ns/m virtual damping
    _stiffness = 100.0f;    // 100 N/m virtual stiffness
    
    // Initialize force values
    _reference_force = 0.0f;    // Will be set from GUI or hardcoded
    _actual_force = 0.0f;
    
    // Initialize kinematic states
    _reference_position = 0.0f;
    _reference_velocity = 0.0f;
    _prev_reference_position = 0.0f;
    _prev_reference_velocity = 0.0f;
    
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
    
    // Get reference force from ExoData (can be set from GUI)
    _reference_force = _data->get_reference_force(_id);
    
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
    // Admittance control equation: M*a + D*v + K*x = F_error
    // Solve for acceleration: a = (F_error - D*v - K*x) / M
    
    float displacement = _reference_position - _prev_reference_position;
    float reference_acceleration = (force_error - _damping * _reference_velocity - _stiffness * displacement) / _mass;
    
    // Integrate to get velocity
    float new_velocity = _reference_velocity + reference_acceleration * dt;
    
    // Apply velocity limits for safety
    const float MAX_VELOCITY = 30.0f;  // 30 deg/s maximum
    new_velocity = constrain(new_velocity, -MAX_VELOCITY, MAX_VELOCITY);
    
    // Integrate to get position
    float new_position = _reference_position + new_velocity * dt;
    
    // Apply position limits for safety
    const float MAX_POSITION = 45.0f;   // 45 degrees maximum
    const float MIN_POSITION = -45.0f;  // -45 degrees minimum
    new_position = constrain(new_position, MIN_POSITION, MAX_POSITION);
    
    // Update states
    _prev_reference_position = _reference_position;
    _prev_reference_velocity = _reference_velocity;
    _reference_position = new_position;
    _reference_velocity = new_velocity;
}

float AdmittanceController::_get_actual_force_from_loadcell()
{
    // Get force reading from the joint's loadcell
    // The force is already processed in the joint data
    float force = 0.0f;
    
    if (_joint_data != nullptr) {
        force = _joint_data->torque_reading;  // This is the processed loadcell reading
    }
    
    return force;
}

//****************************************************
// PositionController Implementation
//****************************************************

PositionController::PositionController(config_defs::joint_id id, ExoData *exo_data)
    : _Controller(id, exo_data)
{
#ifdef CONTROLLER_DEBUG
    Serial.println("PositionController::PositionController");
#endif

    // Initialize PID gains for position control
    _kp_pos = 2.0f;     // Proportional gain
    _ki_pos = 0.1f;     // Integral gain  
    _kd_pos = 0.05f;    // Derivative gain
    
    // Initialize position tracking
    _target_position = 0.0f;
    _current_position = 0.0f;
    
    // Initialize swing trajectory parameters
    _swing_start_position = 0.0f;
    _swing_target_position = 30.0f;  // 30 degrees knee flexion during swing
}

float PositionController::calc_motor_cmd()
{
    // Get current position from joint encoder
    if (_joint_data != nullptr) {
        _current_position = _joint_data->position;
    }
    
    // Calculate target position based on swing trajectory
    _target_position = _calculate_swing_trajectory();
    
    // Use PID control to calculate position command
    float position_cmd = _pid(_target_position, _current_position, _kp_pos, _ki_pos, _kd_pos);
    
    // Apply position limits for safety
    position_cmd = constrain(position_cmd, -60.0f, 60.0f);
    
#ifdef CONTROLLER_DEBUG
    static unsigned long last_debug_time = 0;
    if (millis() - last_debug_time > 500) {  // Debug every 0.5 second
        Serial.print("Position - Target: ");
        Serial.print(_target_position);
        Serial.print("°, Current: ");
        Serial.print(_current_position);
        Serial.print("°, Cmd: ");
        Serial.print(position_cmd);
        Serial.println("°");
        last_debug_time = millis();
    }
#endif
    
    return position_cmd;
}

float PositionController::_calculate_swing_trajectory()
{
    // Get the side data to access gait percentage
    bool is_left = utils::get_is_left(_id);
    SideData* side_data = is_left ? &(_data->left_side) : &(_data->right_side);
    
    float percent_gait = side_data->percent_gait;
    
    // Calculate swing trajectory using a smooth curve
    // Early swing: rapid flexion (0-30%)
    // Mid swing: maintain flexion (30-60%)  
    // Late swing: extension to prepare for heel strike (60-100%)
    
    float trajectory_position = 0.0f;
    
    if (side_data->is_early_swing()) {
        // Rapid knee flexion during early swing
        float t = percent_gait / 30.0f;  // Normalize to 0-1
        trajectory_position = _swing_start_position + _swing_target_position * sin(t * M_PI / 2);
    }
    else if (side_data->is_mid_swing()) {
        // Maintain flexion during mid swing
        trajectory_position = _swing_target_position;
    }
    else if (side_data->is_late_swing()) {
        // Extension during late swing
        float t = (percent_gait - 60.0f) / 40.0f;  // Normalize to 0-1
        trajectory_position = _swing_target_position * (1.0f - t);
    }
    else {
        // Default to start position if not in swing
        trajectory_position = _swing_start_position;
    }
    
    return trajectory_position;
}

//****************************************************
// SpeedController Implementation  
//****************************************************

SpeedController::SpeedController(config_defs::joint_id id, ExoData *exo_data)
    : _Controller(id, exo_data)
{
#ifdef CONTROLLER_DEBUG
    Serial.println("SpeedController::SpeedController");
#endif

    // Initialize PID gains for speed control
    _kp_speed = 1.5f;   // Proportional gain
    _ki_speed = 0.05f;  // Integral gain
    _kd_speed = 0.02f;  // Derivative gain
    
    // Initialize speed tracking
    _target_speed = 0.0f;
    _current_speed = 0.0f;
    
    // Initialize swing speed profile parameters
    _max_swing_speed = 120.0f;      // 120 deg/s maximum speed
    _acceleration_phase = 0.3f;     // First 30% is acceleration
    _deceleration_phase = 0.7f;     // Last 30% is deceleration
}

float SpeedController::calc_motor_cmd()
{
    // Get current speed from joint encoder (derivative of position)
    if (_joint_data != nullptr) {
        static float prev_position = _joint_data->position;
        static unsigned long prev_time = millis();
        
        unsigned long current_time = millis();
        float dt = (current_time - prev_time) / 1000.0f;
        
        if (dt > 0.001f) {  // Avoid division by zero
            _current_speed = (_joint_data->position - prev_position) / dt;
            prev_position = _joint_data->position;
            prev_time = current_time;
        }
    }
    
    // Calculate target speed based on swing speed profile
    _target_speed = _calculate_swing_speed_profile();
    
    // Use PID control to calculate speed command
    float speed_cmd = _pid(_target_speed, _current_speed, _kp_speed, _ki_speed, _kd_speed);
    
    // Apply speed limits for safety
    speed_cmd = constrain(speed_cmd, -150.0f, 150.0f);
    
#ifdef CONTROLLER_DEBUG
    static unsigned long last_debug_time = 0;
    if (millis() - last_debug_time > 500) {  // Debug every 0.5 second
        Serial.print("Speed - Target: ");
        Serial.print(_target_speed);
        Serial.print("°/s, Current: ");
        Serial.print(_current_speed);
        Serial.print("°/s, Cmd: ");
        Serial.print(speed_cmd);
        Serial.println("°/s");
        last_debug_time = millis();
    }
#endif
    
    return speed_cmd;
}

float SpeedController::_calculate_swing_speed_profile()
{
    // Get the side data to access gait percentage
    bool is_left = utils::get_is_left(_id);
    SideData* side_data = is_left ? &(_data->left_side) : &(_data->right_side);
    
    float percent_gait = side_data->percent_gait;
    float profile_speed = 0.0f;
    
    if (side_data->is_swing_phase()) {
        float t = percent_gait / 100.0f;  // Normalize to 0-1
        
        if (t <= _acceleration_phase) {
            // Acceleration phase: speed increases
            float accel_t = t / _acceleration_phase;
            profile_speed = _max_swing_speed * accel_t;
        }
        else if (t <= _deceleration_phase) {
            // Constant speed phase
            profile_speed = _max_swing_speed;
        }
        else {
            // Deceleration phase: speed decreases
            float decel_t = (t - _deceleration_phase) / (1.0f - _deceleration_phase);
            profile_speed = _max_swing_speed * (1.0f - decel_t);
        }
        
        // Apply swing direction (flexion is positive for knee)
        if (side_data->is_early_swing()) {
            profile_speed = abs(profile_speed);  // Flexion direction
        } else if (side_data->is_late_swing()) {
            profile_speed = -abs(profile_speed); // Extension direction
        }
    }
    else {
        // No movement during stance
        profile_speed = 0.0f;
    }
    
    return profile_speed;
}

#endif