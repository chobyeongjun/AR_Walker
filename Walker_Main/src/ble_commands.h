#ifndef BLE_COMMANDS_H
#define BLE_COMMANDS_H

#include "Arduino.h"
#include "ExoData.h"
#include "ParseIni.h"       //For config_defs
#include "StatusDefs.h"     //For ExoDataStatus
#include "BleMessage.h"
#include "ParamsFromSD.h"

#include "UARTHandler.h"
#include "uart_commands.h"
#include "UART_msg_t.h"
#include "Logger.h"

typedef struct
{
    char command;
    int length; 
} ble_command_t;

namespace ble_names
{
    //Recieved Commands (GUI->Firmware)
    static const char start             = 'E';
    static const char stop              = 'G';
    static const char cal_trq           = 'H';
    static const char cal_fsr           = 'L';
    static const char assist            = 'c';
    static const char resist            = 'S';
    static const char motors_on         = 'x';
    static const char motors_off        = 'w';
    static const char mark              = 'N';
    static const char update_param      = 'f';

    //Sending Commands (Firmware->GUI)
    static const char send_real_time_data = '?';
    static const char send_batt           = '~';
    static const char send_error_count    = 'w';
    static const char send_step_count     = 's';

};

namespace ble
{
    static const ble_command_t commands[] = 
    {
        //Recieved Commands
        {ble_names::start,              0},
        {ble_names::stop,               0},
        {ble_names::assist,             0},
        {ble_names::resist,             0},
        {ble_names::motors_on,          0},
        {ble_names::motors_off,         0},
        {ble_names::mark,               0},
        {ble_names::update_param,       4},
        
        //Sending Commands
        {ble_names::send_batt,              1},
        {ble_names::send_real_time_data,    9},
        {ble_names::send_error_count,       1},
        {ble_names::send_step_count,        2},

    };
};


namespace ble_command_helpers
{

    inline static int get_length_for_command(char command)
    {
        int length = -1;

        //Get the amount of characters to wait for
        for(unsigned int i=0; i < sizeof(ble::commands)/sizeof(ble::commands[0]); i++)
        {
            if(command == ble::commands[i].command)
            {
                length = ble::commands[i].length;
                break;
            }
        }
        return length;
    }

}


namespace ble_handler_vars
{
    //Should be used sparingly, we chose to do this so that ExoData wasn't needlessly populated with variables
    static const uint8_t k_max_joints = 6;
    static uint8_t prev_controllers[k_max_joints] = {0, 0, 0, 0, 0, 0};

}


namespace ble_handlers
{
    inline static void start(ExoData* data, BleMessage* msg)
    {
        //Start the trial (ie Enable motors and begin streaming data). If the joint is used; enable the motor, and set the controller to zero torque
        data->for_each_joint(
            
            // This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 1;
                }
                return;
            }
        );

        //Set the data status to running
        data->set_status(status_defs::messages::trial_on);

        //Send status update
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_status;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::status::STATUS] = data->get_status();
        tx_msg.len = (uint8_t)UART_command_enums::status::LENGTH;
        uart_handler->UART_msg(tx_msg);

        delayMicroseconds(10);

        //Send motor enable update
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 1;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);

        delayMicroseconds(10);

    }
    inline static void stop(ExoData* data, BleMessage* msg)
    {
        //Stop the trial (inverse of start) & send trial summary data (step information)
        data->for_each_joint(
            
            //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 0;
                }
                return;
            }
        );

        //Set the data status to off
        data->set_status(status_defs::messages::trial_off);

        //Send status update
        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_status;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::status::STATUS] = data->get_status();
        tx_msg.len = (uint8_t)UART_command_enums::status::LENGTH;
        uart_handler->UART_msg(tx_msg);

        delayMicroseconds(100);

        //Send motor enable update
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 0;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);

        data->mark = 10;
    }


    inline static void assist(ExoData* data, BleMessage* msg)
    {
        //Right now we are approaching defining assistance and resistance directly in the controllers via a controller specific parameter, future work may populate these functions instead
    }
    inline static void resist(ExoData* data, BleMessage* msg)
    {
        //Right now we are approaching defining assistance and resistance directly in the controllers via a controller specific parameter, future work may populate these functions instead
    }
    
    inline static void motors_on(ExoData* data, BleMessage* msg)
    {        
        data->for_each_joint(
            
            //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 1;
                }
                return;
            }
        );

        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 1;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);
        
    }
    inline static void motors_off(ExoData* data, BleMessage* msg)
    {   
        data->for_each_joint(
            
            //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
            [](JointData* j_data, float* args)
            {
                if (j_data->is_used)
                {
                    j_data->motor.enabled = 0;
                }
                return;
            }
        );

        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_motor_enable_disable;
        tx_msg.joint_id = 0;
        tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 0;
        tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
        uart_handler->UART_msg(tx_msg);

    }
    inline static void mark(ExoData* data, BleMessage* msg)
    {
        //Increment mark variable (Done by sending different data on one of the real time signals, we should raise a flag or inc a var in exo_data)
        data->mark++;
    }
    

    inline static void update_param(ExoData* data, BleMessage* msg)
    {
        //Send UART message to update parameter
        logger::println("ble_handlers::update_param() - Got update param message");
        logger::print("ble_handlers::update_param() - Joint ID: "); logger::println((uint8_t)msg->data[0]);
        logger::print("ble_handlers::update_param() - Controller ID: "); logger::println((uint8_t)msg->data[1]);
        logger::print("ble_handlers::update_param() - Param Index: "); logger::println((uint8_t)msg->data[2]);
        logger::print("ble_handlers::update_param() - Param Value: "); logger::println((uint8_t)msg->data[3]);
        logger::print("New message\n");

        UARTHandler* uart_handler = UARTHandler::get_instance();
        UART_msg_t tx_msg;
        tx_msg.command = UART_command_names::update_controller_param;
        tx_msg.joint_id = (uint8_t) msg->data[0];
        tx_msg.data[(uint8_t)UART_command_enums::controller_param::CONTROLLER_ID] = (uint8_t) msg->data[1];
        tx_msg.data[(uint8_t)UART_command_enums::controller_param::PARAM_INDEX] = (uint8_t) msg->data[2];
        tx_msg.data[(uint8_t)UART_command_enums::controller_param::PARAM_VALUE] = (uint8_t) msg->data[3];
        tx_msg.len = 3;
        uart_handler->UART_msg(tx_msg);
    }

}

#endif