#include "Joint.h"
#include "Time_Helper.h"
#include "Logger.h"
#include "ErrorReporter.h"
#include "error_codes.h"

#define JOINT_DEBUG

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// Initialize the used joint counters that will be used to select the TorqueSensor pin. If you don't do it it won't work.
uint8_t _Joint::left_loadcell_used_count = 0;
uint8_t _Joint::right_loadcell_used_count = 0;

uint8_t _Joint::left_motor_used_count = 0;
uint8_t _Joint::right_motor_used_count = 0;

_Joint::_Joint(config_defs::joint_id id, ExoData *exo_data)
    : _loadcell(id, _Joint::get_loadcell_pin(id, exo_data))
{
#ifdef JOINT_DEBUG
    logger::println("_Joint :: Constructor : entered");
#endif

    logger::print("_Joint::right_loadcell_used_count : ");
    logger::println(_Joint::right_loadcell_used_count);
    logger::print("_Joint::left_loadcell_used_count : ");
    logger::println(_Joint::left_loadcell_used_count);

    _id = id;

    _is_left = utils::get_is_left(_id);

    _data = exo_data;

    // Initialize gait state tracking
    _was_in_swing = false;
    _last_state_change_time = 0;
    
    // Initialize gait-based controllers
    _initialize_gait_controllers();

#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    switch (utils::get_joint_type(_id))
    {
    case (uint8_t)config_defs::joint_id::knee:
        logger::print("Knee");
        break;
    case (uint8_t)config_defs::joint_id::ankle:
        logger::print("Ankle");
        break;
    default:
        break;
    }
    logger::println(" :: Constructor : _data set");
#endif
};

void _Joint::read_data()
{
    _joint_data->position = _joint_data->motor.p / _joint_data->motor.gearing;
    _joint_data->velocity = _joint_data->motor.v / _joint_data->motor.gearing;

    // Read the true torque sensor offset
    _joint_data->loadcell_reading = _loadcell.read();
};

// void _Joint::check_calibration()
// {
//     if (_joint_data->motor.do_zero)
//     {
//         _data->set_status(status_defs::messages::motor_start_up); // Set the status to motor start up while we are zeroing
//         _motor->zero();
//         _joint_data->motor.do_zero = false;
//     }
// };

unsigned int _Joint::get_loadcell_pin(config_defs::joint_id id, ExoData *exo_data)
{

    // First check which joint we are looking at. Then go through and if it is the left or right and if it is used. If it is set return the appropriate pin and increment the counter.
    switch (utils::get_joint_type(id))
    {
    case (uint8_t)config_defs::joint_id::knee:
    {
        if (utils::get_is_left(id) && exo_data->left_side.knee.is_used && exo_data->knee_Loadcell_flag == 1) // Check if the left side is used and we want to use the torque sensor
        {
            if (_Joint::left_loadcell_used_count < logic_micro_pins::num_available_joints) // If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
            {
                return logic_micro_pins::loadcell_left[_Joint::left_loadcell_used_count++];
            }
            else
            {
                return logic_micro_pins::not_connected_pin;
            }
        }
        else if (!(utils::get_is_left(id)) && exo_data->right_side.knee.is_used && exo_data->knee_Loadcell_flag == 1) // Check if the right side is used and we want to use the torque sensor
        {
            if (_Joint::right_loadcell_used_count < logic_micro_pins::num_available_joints) // If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
            {
                return logic_micro_pins::loadcell_right[_Joint::right_loadcell_used_count++];
            }
            else
            {
                return logic_micro_pins::not_connected_pin;
            }
        }
        else // The joint isn't used.
        {
            return logic_micro_pins::not_connected_pin;
        }
        break;
    }
    case (uint8_t)config_defs::joint_id::ankle:
    {
        if (utils::get_is_left(id) && exo_data->left_side.ankle.is_used && exo_data->ankle_Loadcell_flag == 1) 
        {
            if (_Joint::left_loadcell_used_count < logic_micro_pins::num_available_joints) // If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
            {
                return logic_micro_pins::loadcell_left[_Joint::left_loadcell_used_count++];
            }
            else
            {
                return logic_micro_pins::not_connected_pin;
            }
        }
        else if (!(utils::get_is_left(id)) && exo_data->right_side.ankle.is_used && exo_data->ankle_Loadcell_flag == 1) 
        {
            if (_Joint::right_loadcell_used_count < logic_micro_pins::num_available_joints) // If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
            {
                return logic_micro_pins::loadcell_right[_Joint::right_loadcell_used_count++];
            }
            else
            {
                return logic_micro_pins::not_connected_pin;
            }
        }
        else // The joint isn't used.
        {
            return logic_micro_pins::not_connected_pin;
        }
        break;
    }

    default:
    {
        return logic_micro_pins::not_connected_pin;
    }
    }
};

void _Joint::set_motor(_Motor *new_motor)
{
    _motor = new_motor;
};

//*********************************************
KneeJoint::KneeJoint(config_defs::joint_id id, ExoData *exo_data)
    : _Joint(id, exo_data) // <-- Initializer list
      , _step(id, exo_data)
{
#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    logger::println("Knee : Knee Constructor");
#endif

    // Set _joint_data to point to the data specific to this joint.
    if (_is_left)
    {
        _joint_data = &(exo_data->left_side.knee);
    }
    else
    {
        _joint_data = &(exo_data->right_side.knee);
    }

#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    logger::println("Knee : _joint_data set");
#endif

    // Don't need to check side as we assume symmetry and create both side data objects. Setup motor from here as it will be easier to check which motor is used
    if (_joint_data->is_used)
    {
#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::print("Knee : setting motor to ");
#endif

        switch (_data->left_side.knee.motor.motor_type)
        {
        // Using new so the object of the specific motor type persists.
        case (uint8_t)config_defs::motor::AK10:
#ifdef JOINT_DEBUG
            logger::println("AK10");
#endif
            KneeJoint::set_motor(new AK10(id, exo_data));
            break;
        case (uint8_t)config_defs::motor::AK80:
#ifdef JOINT_DEBUG
            logger::println("AK80");
#endif
            KneeJoint::set_motor(new AK80(id, exo_data));
            break;
        case (uint8_t)config_defs::motor::AK60_v1_1:
#ifdef JOINT_DEBUG
            logger::println("AK60 v1.1");
#endif
            KneeJoint::set_motor(new AK60_v1_1(id, exo_data));
            break;
        case (uint8_t)config_defs::motor::AK70:
#ifdef JOINT_DEBUG
            logger::println("AK70");
#endif
            KneeJoint::set_motor(new AK70(id, exo_data));
            break;

        default:
#ifdef JOINT_DEBUG
            logger::println("NULL");
#endif
            KneeJoint::set_motor(new NullMotor(id, exo_data));
            break;
        }

        delay(5);

#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Knee : Setting Controller");
#endif

        set_controller(exo_data->left_side.knee.controller.controller);

#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Knee : _controller set");
#endif
    }
};

void KneeJoint::run_joint()
{
#ifdef JOINT_DEBUG
    logger::print("KneeJoint::run_joint::Start");
#endif

    // === NEW GAIT-BASED CONTROL SYSTEM ===
    
    // 1. Select appropriate controller based on current gait state
    _select_controller_based_on_gait_state();

    // 2. Calculate motor command using selected controller
    float motor_cmd = _controller->calc_motor_cmd();
    _joint_data->controller.setpoint = motor_cmd;

    // 3. Check for joint errors
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on);
    const bool error = correct_status ? _error_manager.run(_joint_data) : false;

    if (error)
    {
        _motor->set_error();
        // Send all errors to the other microcontroller
        for (int i = 0; i < _error_manager.errorQueueSize(); i++)
        {
            ErrorReporter::get_instance()->report(_error_manager.popError(), _id);
        }
    }

    // 4. Turn motor on/off as needed
    _motor->on_off();

    // 5. Apply motor command using appropriate servo mode based on gait state
    _apply_motor_command(motor_cmd);

#ifdef JOINT_DEBUG
    SideData* side_data = _get_side_data();
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 1000) {
        logger::print("Knee - State: ");
        logger::print(side_data->is_swing_phase() ? "SWING" : "STANCE");
        logger::print(", Cmd: ");
        logger::print(motor_cmd);
        logger::println(side_data->is_swing_phase() ? "°" : "° (Admittance)");
        last_debug = millis();
    }
#endif
};

void KneeJoint::set_controller(uint8_t controller_id) // Changes the high level controller in Controller, and the low level controller in Motor
{
#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    logger::println("Knee : set_controller : Entered");
    logger::print("Knee : set_controller : Controller ID : ");
    logger::println(controller_id);
#endif

    switch (controller_id)
    {
    case (uint8_t)config_defs::knee_controllers::step:
        _controller = &_step;
        break;
    default:
        logger::print("Unkown Controller!\n", LogLevel::Error);
        _controller = &_step;
        break;
    }
};

//=================================================================
AnkleJoint::AnkleJoint(config_defs::joint_id id, ExoData *exo_data)
    : _Joint(id, exo_data) // <-- Initializer list
      , _step(id, exo_data)
{
#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    logger::println("Ankle : Ankle Constructor");
#endif

    // Set _joint_data to point to the data specific to this joint.
    if (_is_left)
    {
        _joint_data = &(exo_data->left_side.ankle);
    }
    else
    {
        _joint_data = &(exo_data->right_side.ankle);
    }

#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    logger::println("Ankle : _joint_data set");
#endif

    // Don't need to check side as we assume symmetry and create both side data objects. Setup motor from here as it will be easier to check which motor is used
    if (_joint_data->is_used)
    {
#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::print("Ankle : setting motor to ");
#endif
        switch (_data->left_side.ankle.motor.motor_type)
        {
        // Using new so the object of the specific motor type persists.
        case (uint8_t)config_defs::motor::AK10:
#ifdef JOINT_DEBUG
            logger::println("AK10");
#endif
            AnkleJoint::set_motor(new AK10(id, exo_data));
            break;
        case (uint8_t)config_defs::motor::AK80:
#ifdef JOINT_DEBUG
            logger::println("AK80");
#endif
            AnkleJoint::set_motor(new AK80(id, exo_data));
            break;
        case (uint8_t)config_defs::motor::AK60_v1_1:
#ifdef JOINT_DEBUG
            logger::println("AK60 v1.1");
#endif
            AnkleJoint::set_motor(new AK60_v1_1(id, exo_data));
            break;
        case (uint8_t)config_defs::motor::AK70:
#ifdef JOINT_DEBUG
            logger::println("AK70");
#endif
            AnkleJoint::set_motor(new AK70(id, exo_data));
            break;
        default:
#ifdef JOINT_DEBUG
            logger::println("NULL");
#endif
            AnkleJoint::set_motor(new NullMotor(id, exo_data));
            break;
        }

        delay(5);

#ifdef JOINT_DEBUG
        logger::println("_is_left section");
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Ankle : Setting Controller");
#endif

        set_controller(exo_data->left_side.ankle.controller.controller);

#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Ankle : _controller set");
#endif
    }
};

void AnkleJoint::run_joint()
{
#ifdef JOINT_DEBUG
    logger::print("AnkleJoint::run_joint::Start");
#endif

    // === NEW GAIT-BASED CONTROL SYSTEM ===
    
    // 1. Select appropriate controller based on current gait state
    _select_controller_based_on_gait_state();

    // 2. Calculate motor command using selected controller
    float motor_cmd = _controller->calc_motor_cmd();
    _joint_data->controller.setpoint = motor_cmd;

    // 3. Check for joint errors
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on);
    const bool error = correct_status ? _error_manager.run(_joint_data) : false;

    if (error)
    {
        _motor->set_error();
        _motor->on_off();

        // Send all errors to the other microcontroller
        for (int i = 0; i < _error_manager.errorQueueSize(); i++)
        {
            ErrorReporter::get_instance()->report(_error_manager.popError(), _id);
        }
    }

    // 4. Turn motor on/off as needed
    _motor->on_off();

    // 5. Apply motor command using appropriate servo mode based on gait state
    _apply_motor_command(motor_cmd);

#ifdef JOINT_DEBUG
    SideData* side_data = _get_side_data();
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 1000) {
        logger::print("Ankle - State: ");
        logger::print(side_data->is_swing_phase() ? "SWING" : "STANCE");
        logger::print(", Cmd: ");
        logger::print(motor_cmd);
        logger::println(side_data->is_swing_phase() ? "°" : "° (Admittance)");
        last_debug = millis();
    }
#endif
};

void AnkleJoint::set_controller(uint8_t controller_id) // Changes the high level controller in Controller, and the low level controller in Motor
{
#ifdef JOINT_DEBUG
    logger::print(_is_left ? "Left " : "Right ");
    logger::println("Ankle : set_controller : Entered");
    logger::print("Ankle : set_controller : Controller ID : ");
    logger::println(controller_id);
#endif
    switch (controller_id)
    {
   
    case (uint8_t)config_defs::ankle_controllers::step:
        _controller = &_step;
        break;

    default:
        logger::print("Unkown Controller!\n", LogLevel::Error);
        _controller = &_step;
        break;
    }
};

//****************************************************
// Gait State-Based Control Implementation
//****************************************************

void _Joint::_initialize_gait_controllers()
{
#ifdef JOINT_DEBUG
    logger::println("_Joint::_initialize_gait_controllers");
#endif

    // Initialize swing phase controllers
    _swing_position_controller = new PositionController(_id, _data);
    _swing_speed_controller = new SpeedController(_id, _data);
    
    // Initialize stance phase controller
    _stance_admittance_controller = new AdmittanceController(_id, _data);
    
    // Set default controller to admittance (stance)
    _controller = _stance_admittance_controller;
}

void _Joint::_select_controller_based_on_gait_state()
{
    SideData* side_data = _get_side_data();
    if (side_data == nullptr) return;
    
    bool is_currently_in_swing = side_data->is_swing_phase();
    bool state_changed = (is_currently_in_swing != _was_in_swing);
    
    // Check for gait state transitions
    if (state_changed) {
        _last_state_change_time = millis();
        
#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::print(" Joint: Gait state changed to ");
        logger::println(is_currently_in_swing ? "SWING" : "STANCE");
#endif
    }
    
    // Select controller based on gait state
    if (is_currently_in_swing) {
        // SWING PHASE: Use Position or Speed Control
        
        if (side_data->is_early_swing() || side_data->is_late_swing()) {
            // Use position control during early and late swing for precision
            if (_controller != _swing_position_controller) {
                _controller = _swing_position_controller;
                
#ifdef JOINT_DEBUG
                logger::println("Switched to Position Controller (Swing)");
#endif
            }
        } 
        else if (side_data->is_mid_swing()) {
            // Use speed control during mid swing for smooth motion
            if (_controller != _swing_speed_controller) {
                _controller = _swing_speed_controller;
                
#ifdef JOINT_DEBUG
                logger::println("Switched to Speed Controller (Swing)");
#endif
            }
        }
    }
    else {
        // STANCE PHASE: Use Admittance Control
        if (_controller != _stance_admittance_controller) {
            _controller = _stance_admittance_controller;
            
#ifdef JOINT_DEBUG
            logger::println("Switched to Admittance Controller (Stance)");
#endif
        }
    }
    
    // Update state tracking
    _was_in_swing = is_currently_in_swing;
}

void _Joint::_apply_motor_command(float motor_cmd)
{
    SideData* side_data = _get_side_data();
    if (side_data == nullptr || _motor == nullptr) return;
    
    if (side_data->is_swing_phase()) {
        // SWING PHASE: Use servo position/speed commands
        
        if (_controller == _swing_position_controller) {
            // Position control mode
            _motor->set_position(motor_cmd);
            
#ifdef JOINT_DEBUG
            static unsigned long last_debug = 0;
            if (millis() - last_debug > 200) {
                logger::print("Swing Position Cmd: ");
                logger::print(motor_cmd);
                logger::println("°");
                last_debug = millis();
            }
#endif
        }
        else if (_controller == _swing_speed_controller) {
            // Speed control mode  
            _motor->set_speed(motor_cmd);
            
#ifdef JOINT_DEBUG
            static unsigned long last_debug = 0;
            if (millis() - last_debug > 200) {
                logger::print("Swing Speed Cmd: ");
                logger::print(motor_cmd);
                logger::println("°/s");
                last_debug = millis();
            }
#endif
        }
    }
    else {
        // STANCE PHASE: Use admittance control (position-based)
        _motor->set_position(motor_cmd);
        
#ifdef JOINT_DEBUG
        static unsigned long last_debug = 0;
        if (millis() - last_debug > 200) {
            logger::print("Stance Admittance Cmd: ");
            logger::print(motor_cmd);
            logger::println("°");
            last_debug = millis();
        }
#endif
    }
}

SideData* _Joint::_get_side_data()
{
    if (_data == nullptr) return nullptr;
    
    return _is_left ? &(_data->left_side) : &(_data->right_side);
}

#endif
