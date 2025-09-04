#ifndef Joint_h
#define Joint_h

// Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "ExoData.h"
#include "Motor.h"
#include "Controller.h"
#include "Loadcell.h"
#include "ParseIni.h"
#include "board.h"
#include "config.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include "ErrorManager.h"

#include <stdint.h>

class _Joint
{
    static uint8_t left_loadcell_used_count; 
    static uint8_t right_loadcell_used_count;

    static uint8_t left_motor_used_count;  
    static uint8_t right_motor_used_count; 

public:

    _Joint(config_defs::joint_id id, ExoData *exo_data); // Constructor:
    virtual ~_Joint() {};

    virtual void run_joint() = 0;

    virtual void read_data();

    virtual void set_motor_position_zero();

    virtual void set_controller(uint8_t) = 0;

    void set_motor(_Motor *new_motor);

    static unsigned int get_loadcell_pin(config_defs::joint_id, ExoData *);

    _Motor *_motor;              /**< Pointer to the base _Motor class so we can use any motor type.*/
    Loadcell _loadcell; 
    _Controller *_controller;    /**< Pointer to the current controller. Using pointer so we just need to change the object we are pointing to when the controller changes.*/
    
    // Gait-based controllers
    _Controller *_swing_position_controller;  /**< Position controller for swing phase */
    _Controller *_swing_speed_controller;     /**< Speed controller for swing phase */
    _Controller *_stance_admittance_controller; /**< Admittance controller for stance phase */

protected:
    // Give access to the larger data object and the joint specific data
    ExoData *_data;              /**< Pointer to the full data instance*/
    JointData *_joint_data;      /**< Pointer to this joints data */
    ErrorManager _error_manager; /**< Error manager for the joint */

    // Joint info
    config_defs::joint_id _id; /**< Joint id */
    bool _is_left;             /**< If the joint is on the left side so we don't have to keep calculating it */
    
    // Gait state-based control methods
    void _initialize_gait_controllers();                    /**< Initialize all gait-based controllers */
    void _select_controller_based_on_gait_state();          /**< Switch controller based on current gait state */
    void _apply_motor_command(float motor_cmd);              /**< Apply motor command using appropriate servo mode */
    SideData* _get_side_data();                             /**< Get the appropriate side data */
    
    // Controller switching state tracking
    bool _was_in_swing;                                      /**< Track previous gait state for smooth transitions */
    unsigned long _last_state_change_time;                  /**< Time of last gait state change */
};

class KneeJoint : public _Joint
{
public:
    KneeJoint(config_defs::joint_id id, ExoData *exo_data);
    ~KneeJoint() {};

    void run_joint();

    void set_controller(uint8_t);

protected:
   
    Step _step;                      /**< Step Controller for Device Characterization */
};


class AnkleJoint : public _Joint
{
public:
    AnkleJoint(config_defs::joint_id id, ExoData *exo_data);
    ~AnkleJoint() {};

    void run_joint();

    void set_controller(uint8_t);

protected:
  
    Step _step;                                         /**< Step Controller for Device Characterization */
  
};


#endif
#endif