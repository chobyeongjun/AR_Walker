#ifndef BOARD_SETTINGS_HEADER
#define BOARD_SETTINGS_HEADER

#include "Config.h"

#if BOARD_VERSION == AK_Board_V0_5_1
#include "Arduino.h"
namespace logic_micro_pins // Teensy
{
#if defined(ARDUINO_TEENSY41)
    // Serial Pins
    const unsigned int not_connected_pin = 51;

    const unsigned int rx1_pin = 0;
    const unsigned int tx1_pin = 1;

    // IMU Serial Pins (Serial4)
    const unsigned int imu_rx_pin = 16;                // RX4 핀
    const unsigned int imu_tx_pin = not_connected_pin; // TX4 핀

    // CAN Pins
    const unsigned int can_rx_pin = 23;
    const unsigned int can_tx_pin = 22;

    // Loadcell Pins
    const unsigned int num_available_joints = 2;
    const unsigned int loadcell_left[] = {A16};
    const unsigned int loadcell_right[] = {A6};

    // Sync LED Pins
    const unsigned int sync_led_pin = 15;
    const unsigned int sync_default_pin = 5;

    // Status LED Pins
    const unsigned int status_led_r_pin = 14;
    const unsigned int status_led_g_pin = 25;
    const unsigned int status_led_b_pin = 24;

    // SPI Follower Pins
    const unsigned int miso_pin = not_connected_pin;
    const unsigned int mosi_pin = 11;
    const unsigned int sck_pin = not_connected_pin;
    const unsigned int cs_pin = 10;
    const unsigned int irq_pin = 34;
    const unsigned int rst_pin = 4;
    const unsigned int spi_mode = 8;

    // Pin to Stop the Motors
    const unsigned int motor_stop_pin = 9;


    const unsigned int left_ankle_angle_pin = A13;
    const unsigned int right_ankle_angle_pin = A12;

    // Pin to use when we need a value but don't actually want to use it.
#endif

    const unsigned int sync_led_on_state = LOW;
    const unsigned int sync_led_off_state = HIGH;
    const bool status_has_pwm = true;
    const uint8_t status_led_on_state = 0;
        const uint16_t status_led_off_state = 4095;
    const unsigned int motor_enable_on_state = HIGH;
    const unsigned int motor_enable_off_state = LOW;
};

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
namespace coms_micro_pins // Nano
{
    const unsigned int blue = 24;
    const unsigned int green = 23;
    const unsigned int red = 22;
    const unsigned int led_active_low = 1;

    const unsigned int miso_pin = 11;
    const unsigned int mosi_pin = 12;
    const unsigned int sck_pin = 13;
    const unsigned int cs_pin = 10;
    const unsigned int spi_mode = 16;
};
#endif

#endif

#endif // BOARD_SETTINGS_HEADER
