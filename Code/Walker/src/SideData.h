#ifndef SideData_h
#define SideData_h

#include "Arduino.h"
#include "ParseIni.h"
#include "Board.h"
#include <stdint.h>

class ExoData;

class SideData
{

public:
    SideData(bool is_left, uint8_t *config_to_send);
    void reconfigure(uint8_t *config_to_send);

    bool is_left; /**< 1 if the side is on the left, 0 otherwise */
    bool is_used; /**< 1 if the side is used, 0 otherwise */

    // Loadcell Data
    float loadcell_bias_knee;
    float loadcell_sensitive_knee;
    float loadcell_bias_ankle;
    float loadcell_sensitive_ankle;

    // IMU Data
    float imu_acc_x, imu_acc_y, imu_acc_z;    /**< IMU accelerometer data */
    float imu_gyro_x, imu_gyro_y, imu_gyro_z; /**< IMU gyroscope data */
    float imu_roll, imu_pitch, imu_yaw;       /**< IMU angle data */
    uint8_t imu_id_knee;
    uint8_t imu_id_ankle;

    // Gait Cycle Variables
    float percent_gait;           /**< Estimate of the percent gait based on IMU data */
    float expected_gait_duration; /**< Estimate of how long the next gait cycle will take */
    uint32_t toe_off_timestamp;   /**< Timestamp of the last detected toe off event */

    // EWMA Filter
    float ewma_alpha;
};

#endif