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
#include "Joint.h"
#include "config.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include "ErrorManager.h"

#include <stdint.h>

class _Joint
{
    static uint8_t left_loadcell_used_count;  /**< Used to record how many sensors are already set */
    static uint8_t right_loadcell_used_count; /**< Used to record how many sensors are already set */

    static uint8_t left_motor_used_count;  /**< Used to record how many motors are already set for torque sensor pin assignment */
    static uint8_t right_motor_used_count; /**< Used to record how many motors are already set for torque sensor pin assignment */

public:

    _Joint(config_defs::joint_id id, ExoData *exo_data); // Constructor:
    virtual ~_Joint() {};

    virtual void run_joint() = 0;

    virtual void read_data();

    virtual void set_motor_position_zero();

    virtual void set_controller(uint8_t) = 0;

    void set_motor(_Motor *new_motor);

    static unsigned int get_loadcell_pin(config_defs::joint_id, ExoData *);

    static unsigned int get_motor_enable_pin(config_defs::joint_id, ExoData *);

    _Motor *_motor;              /**< Pointer to the base _Motor class so we can use any motor type.*/
    Loadcell _loadcell; 
    _Controller *_controller;    /**< Pointer to the current controller. Using pointer so we just need to change the object we are pointing to when the controller changes.*/

protected:
    // Give access to the larger data object and the joint specific data
    ExoData *_data;              /**< Pointer to the full data instance*/
    JointData *_joint_data;      /**< Pointer to this joints data */
    ErrorManager _error_manager; /**< Error manager for the joint */

    // Joint info
    config_defs::joint_id _id; /**< Joint id */
    bool _is_left;             /**< If the joint is on the left side so we don't have to keep calculating it */
};

class KneeJoint : public _Joint
{
public:
    KneeJoint(config_defs::joint_id id, ExoData *exo_data);
    ~KneeJoint() {};

    void run_joint();

    void set_controller(uint8_t);

protected:
    // Objects for joint specific controllers
    ZeroTorque _zero_torque;         /**< Zero torque controller */
    ConstantTorque _constant_torque; /**< Constant torque controller */
    Chirp _chirp;                    /**< Chirp Controller for Device Characterization */
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

    // Objects for joint specific controllers
    ZeroTorque _zero_torque;                            /**< Zero torque controller */
    ProportionalJointMoment _proportional_joint_moment; /**< Proportional joint moment controller */
    ZhangCollins _zhang_collins;                        /**< Zhang Collins controller */
    ConstantTorque _constant_torque;                    /**< Constant torque controller*/
    TREC _trec;                                         /**< TREC */
    CalibrManager _calibr_manager;                      /**< Calibration Manager "Controller" */
    Chirp _chirp;                                       /**< Chirp Controller for Device Characterization */
    Step _step;                                         /**< Step Controller for Device Characterization */
    SPV2 _spv2;                                         /**< SPV2 */
    PJMC_PLUS _pjmc_plus;                               /**< The new proportional joint moment controller */
};


#endif
#endif