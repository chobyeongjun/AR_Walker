#ifndef Joint_h
#define Joint_h

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include "Arduino.h"
#include "ExoData.h"
#include "Motor.h"
#include "Controller.h"
#include "ParseIni.h"
#include "Board.h"
#include "JointData.h"
#include "Loadcell.h"
#include "config.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include "ErrorManager.h"

#include <stdint.h>

class _Joint
{
public:
    _Joint(config_defs::joint_id id, ExoData *exo_data);
    virtual ~_Joint() {};

    virtual void run_joint() = 0;
    virtual void read_data();

    // 캘리브레이션을 외부에서 하므로, check_calibration() 함수는 제거
    // virtual void check_calibration();

    virtual void set_controller(uint8_t) = 0;

    void set_motor(_Motor *new_motor);

    static unsigned int get_loadcell_pin(config_defs::joint_id, ExoData *);
    static unsigned int get_motor_enable_pin(config_defs::joint_id, ExoData *);

    _Motor *_motor;
    Loadcell _loadcell;
    _Controller *_controller;

protected:
    ExoData *_data;
    JointData *_joint_data;
    ErrorManager _error_manager;

    config_defs::joint_id _id;
    bool _is_left;
};

/**
 * @brief Class for the knee joint which contains joint specific controllers.
 */
class KneeJoint : public _Joint
{
public:
    KneeJoint(config_defs::joint_id id, ExoData *exo_data);
    ~KneeJoint() {};
    void run_joint();
    void set_controller(uint8_t);
};

/**
 * @brief Class for the ankle joint which contains joint specific controllers.
 */
class AnkleJoint : public _Joint
{
public:
    AnkleJoint(config_defs::joint_id id, ExoData *exo_data);
    ~AnkleJoint() {};
    void run_joint();
    void set_controller(uint8_t);
};

#endif
#endif
