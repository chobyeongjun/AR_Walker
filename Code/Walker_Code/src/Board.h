#ifndef BOARD_SETTINGS_HEADER
#define BOARD_SETTINGS_HEADER

#include "Config.h"

#if BOARD_VERSION == AK_Board_V0_5_1
   
#include "Arduino.h"
    namespace logic_micro_pins   //Teensy
{
#if defined(ARDUINO_TEENSY41)
    // Serial Pins, NC
    const unsigned int rx1_pin = 0;
    const unsigned int tx1_pin = 1;

    // CAN Pins
    const unsigned int can_rx_pin = 23;
    const unsigned int can_tx_pin = 22;

    // FSR Pins
    const unsigned int fsr_sense_left_heel_pin = A14;
    const unsigned int fsr_sense_left_toe_pin = A15;
    const unsigned int fsr_sense_right_heel_pin = A3;
    const unsigned int fsr_sense_right_toe_pin = A2;

    // Torque Sensor Pins
    const unsigned int num_available_joints = 2;
    // const unsigned int torque_sensor_left[] = {A16, A17};
    const unsigned int torque_sensor_left[] = {A16};
    // const unsigned int torque_sensor_left1 = A16;
    // const unsigned int torque_sensor_right[] = {A6, A7};
    const unsigned int torque_sensor_right[] = {A6};
    // const unsigned int torque_sensor_right1 = A8;

    // Sync LED Pins
    const unsigned int sync_led_pin = 15;
    const unsigned int sync_default_pin = 5;

    // For Maxon PCB only
    const unsigned int maxon_err_right_pin;
    const unsigned int maxon_err_left_pin;
    const unsigned int maxon_ctrl_left_pin;
    const unsigned int maxon_ctrl_right_pin;
    const unsigned int maxon_current_left_pin;
    const unsigned int maxon_current_right_pin;
    const unsigned int maxon_pwm_neutral_val;
    const unsigned int maxon_pwm_u_bound;
    const unsigned int maxon_pwm_l_bound;
#endif

    // Arduino compiles all files not just the ones that are used so this is not under teensy to prevent errors
    const unsigned int sync_led_on_state = LOW;
    const unsigned int sync_led_off_state = HIGH;

#if defined(ARDUINO_TEENSY41)
    // Status LED Pins
    const unsigned int status_led_r_pin = 14;
    const unsigned int status_led_g_pin = 25;
    const unsigned int status_led_b_pin = 24;
#endif

    // If you have connected to pins with PWM set to true.
    const bool status_has_pwm = true;

    // For high to be on use 255 for the on state and 0 for the off, for low as on flip it.
    const uint8_t status_led_on_state = 0;     // 255;
    const uint8_t status_led_off_state = 4095; // 0;

#if defined(ARDUINO_TEENSY41)    
    // SPI Follower Pins
    const unsigned int miso_pin = 12;
    const unsigned int mosi_pin = 11;
    const unsigned int sck_pin = 13;
    const unsigned int cs_pin = 10;
    const unsigned int irq_pin = 34;
    const unsigned int rst_pin = 4;
    const unsigned int spi_mode = 8;

    // Pin to Stop the Motors
    const unsigned int motor_stop_pin = 9;

    // Pin to use when we need a value but don't actually want to use it.
    const unsigned int not_connected_pin = 51;

    // Motor enable Pins
    const unsigned int enable_left_pin[] = {28, 29};
    const unsigned int enable_right_pin[] = {8, 7};

    const unsigned int speed_check_pin = 33;

    const unsigned int left_ankle_angle_pin = A13;
    const unsigned int right_ankle_angle_pin = A12;
#endif
};

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
namespace coms_micro_pins   //Nano
{
    const unsigned int blue = 24;
    const unsigned int green = 23;
    const unsigned int red = 22;
    const unsigned int led_active_low = 1;

    // SPI Conroller Pins
    const unsigned int miso_pin = 11;
    const unsigned int mosi_pin = 12;
    const unsigned int sck_pin = 13;
    const unsigned int cs_pin = 10;
    const unsigned int spi_mode = 16;
};
#endif

#endif
