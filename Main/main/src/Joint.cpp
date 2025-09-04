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

    // Initialize admittance controller
    _initialize_admittance_controller();

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
      ,
      _step(id, exo_data)
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

    // === SIMPLIFIED ADMITTANCE CONTROL SYSTEM ===

    // 1. Update reference force based on gait state (Stance: Half Sine, Swing: 0N)
    _update_reference_force();

    // 2. Calculate motor command using admittance controller
    float motor_cmd = _controller->calc_motor_cmd();
    _joint_data->controller.setpoint = motor_cmd;

    // 3. Check for joint errors
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on);
    const bool error = correct_status ? _error_manager.run(_joint_data, _data) : false;

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

    // 5. Check for error-triggered brake requests
    if (_joint_data->brake_requested)
    {
        // Apply requested brake current
        float brake_current = _joint_data->brake_current;
        _motor->set_current_brake(0.0f, brake_current);

        // Reset brake request flag
        _joint_data->brake_requested = false;
        _joint_data->brake_current = 0.0f;

#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::print(" Knee: ERROR BRAKE APPLIED - ");
        logger::print(brake_current);
        logger::println("A");
#endif
        return; // Skip normal motor commands during brake
    }

    // 6. Apply motor command using appropriate servo mode based on gait state
    _apply_motor_command(motor_cmd);

#ifdef JOINT_DEBUG
    SideData *side_data = _get_side_data();
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 1000)
    {
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
      ,
      _step(id, exo_data)
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

    // === SIMPLIFIED ADMITTANCE CONTROL SYSTEM ===

    // 1. Update reference force based on gait state (Stance: Half Sine, Swing: 0N)
    _update_reference_force();

    // 2. Calculate motor command using admittance controller
    float motor_cmd = _controller->calc_motor_cmd();
    _joint_data->controller.setpoint = motor_cmd;

    // 3. Check for joint errors
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on);
    const bool error = correct_status ? _error_manager.run(_joint_data, _data) : false;

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

    // 5. Check for error-triggered brake requests
    if (_joint_data->brake_requested)
    {
        // Apply requested brake current
        float brake_current = _joint_data->brake_current;
        _motor->set_current_brake(0.0f, brake_current);

        // Reset brake request flag
        _joint_data->brake_requested = false;
        _joint_data->brake_current = 0.0f;

#ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::print(" Ankle: ERROR BRAKE APPLIED - ");
        logger::print(brake_current);
        logger::println("A");
#endif
        return; // Skip normal motor commands during brake
    }

    // 6. Apply motor command using appropriate servo mode based on gait state
    _apply_motor_command(motor_cmd);

#ifdef JOINT_DEBUG
    SideData *side_data = _get_side_data();
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 1000)
    {
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

void _Joint::_initialize_admittance_controller()
{
#ifdef JOINT_DEBUG
    logger::println("_Joint::_initialize_admittance_controller");
#endif

    // Initialize single admittance controller for all gait phases
    _admittance_controller = new AdmittanceController(_id, _data);

    // Set controller to admittance (used for both stance and swing)
    _controller = _admittance_controller;
}

void _Joint::_update_reference_force()
{
    SideData *side_data = _get_side_data();
    if (side_data == nullptr)
        return;

    float reference_force = 0.0f;

    // Set reference force based on gait state
    if (side_data->is_swing_phase())
    {
        // SWING PHASE: Zero force (allow free movement)
        reference_force = 0.0f;

#ifdef JOINT_DEBUG
        static unsigned long last_debug = 0;
        if (millis() - last_debug > 500)
        {
            logger::print(_is_left ? "Left " : "Right ");
            logger::println(" Joint: SWING - Zero Force");
            last_debug = millis();
        }
#endif
    }
    else
    {
        // STANCE PHASE: Use calculated reference force (Half Sine profile)
        reference_force = side_data->current_reference_force;

#ifdef JOINT_DEBUG
        static unsigned long last_debug = 0;
        if (millis() - last_debug > 500)
        {
            logger::print(_is_left ? "Left " : "Right ");
            logger::print(" Joint: STANCE - Force: ");
            logger::print(reference_force);
            logger::println("N");
            last_debug = millis();
        }
#endif
    }

    // Update ExoData with new reference force for this joint
    // AdmittanceController will read this via _data->get_reference_force(_id)
    _data->set_reference_force(_id, reference_force);
}

void _Joint::_apply_motor_command(float motor_cmd)
{
    SideData *side_data = _get_side_data();
    SideData *opposite_side = _get_opposite_side_data();
    if (side_data == nullptr || opposite_side == nullptr || _motor == nullptr)
        return;

    // SAFETY CHECK: Normal gait pattern validation
    bool abnormal_gait = false;
    if (side_data->is_swing_phase() && opposite_side->is_swing_phase())
    {
        abnormal_gait = true;
#ifdef JOINT_DEBUG
        static unsigned long last_safety_debug = 0;
        if (millis() - last_safety_debug > 2000)
        {
            logger::print("SAFETY WARNING: Both legs in SWING phase - ");
            logger::println(_is_left ? "Left Joint" : "Right Joint");
            last_safety_debug = millis();
        }
#endif
    }

    // 비정상 보행 패턴일 때 안전 모드로 전환
    if (abnormal_gait)
    {
        _motor->set_current_brake(0.0f, 1.0f);
        return; // 정상 제어 로직 스킵
    }

    // Determine control mode based on gait phase and assistance requirements
    if (side_data->is_swing_phase() && side_data->assistance_active)
    {
        _motor->set_position(motor_cmd);

#ifdef JOINT_DEBUG
        static unsigned long last_swing_debug = 0;
        if (millis() - last_swing_debug > 500)
        {
            logger::print(_is_left ? "Left " : "Right ");
            logger::print(" SWING Position: ");
            logger::print(motor_cmd);
            logger::print("° (Force: ");
            logger::print(side_data->current_reference_force);
            logger::println("N)");
            last_swing_debug = millis();
        }
#endif
    }

    else if (!side_data->is_swing_phase())
    {
        float stance_current = _joint_data->stance_current; // User configurable value for continuous pulling
        _motor->set_current(stance_current);

#ifdef JOINT_DEBUG
        static unsigned long last_stance_debug = 0;
        if (millis() - last_stance_debug > 1000)
        {
            logger::print(_is_left ? "Left " : "Right ");
            logger::print(" STANCE Current: ");
            logger::print(stance_current);
            logger::print("A (Continuous Pull)");
            logger::println();
            last_stance_debug = millis();
        }
#endif
    }
    else
    {
        // SWING PHASE WITHOUT ASSISTANCE: Transparent position control
        _motor->set_position(motor_cmd);

#ifdef JOINT_DEBUG
        static unsigned long last_transparent_debug = 0;
        if (millis() - last_transparent_debug > 1000)
        {
            logger::print(_is_left ? "Left " : "Right ");
            logger::print(" SWING Transparent: ");
            logger::print(motor_cmd);
            logger::println("°");
            last_transparent_debug = millis();
        }
#endif
    }
}

SideData *_Joint::_get_side_data()
{
    if (_data == nullptr)
        return nullptr;

    return _is_left ? &(_data->left_side) : &(_data->right_side);
}

SideData *_Joint::_get_opposite_side_data()
{
    if (_data == nullptr)
        return nullptr;

    return _is_left ? &(_data->right_side) : &(_data->left_side);
}

#endif
