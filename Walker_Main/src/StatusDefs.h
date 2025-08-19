#ifndef StatusDefs_h
#define StatusDefs_h

#include "Arduino.h"
#include "Board.h"

namespace status_defs
{
    namespace messages
    {

       const uint16_t off =  0;                /**< Off status id */
        const uint16_t trial_off = 1;           /**< Trial off status id */
        const uint16_t trial_on = 2;            /**< Trial on status id */
        const uint16_t test = 3;                /**< Test status id, generally won't be used.*/
        const uint16_t motor_start_up =4;       /**< Motor startup status id */

        //Specific error messages will use the 4 highest bits, giving 31 error messages
        const uint16_t error_bit = 4;               /**< Bit to indicate an error, gives 12 bits of error messages*/ 
        const uint16_t error = 1<<(error_bit-1);    /**< General error message status id */
    
        const uint16_t error_left_knee_loadcell =  1<<error_bit | error;      
        const uint16_t error_right_knee_loadcell =  2<<error_bit | error;      
        const uint16_t error_left_ankle_loadcell =  3<<error_bit | error;      
        const uint16_t error_right_ankle_loadcell = 4<< error_bit | error;     
        const uint16_t error_left_knee_imu  =  5<<error_bit | error;     
        const uint16_t error_right_knee_imu =  6<<error_bit | error;     
        const uint16_t error_left_ankle_imu  =  7<<error_bit | error;  
        const uint16_t error_right_ankle_imu = 8<< error_bit | error;   
        
        const uint16_t error_left_knee_motor =  9<<error_bit | error;              /**< Left knee motor error id */
        const uint16_t error_left_ankle_motor =  10<<error_bit | error;             /**< Left ankle motor error id */
        const uint16_t error_right_knee_motor =  11<<error_bit | error;             /**< Right knee motor error id */
        const uint16_t error_right_ankle_motor =  12<<error_bit | error;            /**< Right ankle motor error id */
        
        const uint16_t error_left_knee_controller =  13<<error_bit | error;         /**< Left knee controller error id */
        const uint16_t error_left_ankle_controller =  14<<error_bit | error;        /**< Left ankle controller error id */
        const uint16_t error_right_knee_controller =  15<<error_bit | error;        /**< Right knee controller error id */
        const uint16_t error_right_ankle_controller =  16<<error_bit | error;       /**< Right ankle controller error id */
        
        
        const uint16_t error_to_be_used_1 =  17<<error_bit | error;     /**< Placeholder error id */
        const uint16_t error_to_be_used_2 =  18<<error_bit | error;     /**< Placeholder error id */
        const uint16_t error_to_be_used_3 =  19<<error_bit | error;     /**< Placeholder error id */
        const uint16_t error_to_be_used_4 =  20<<error_bit | error;    /**< Placeholder error id */
        const uint16_t error_to_be_used_5 =  21<<error_bit | error;    /**< Placeholder error id */
        const uint16_t error_to_be_used_6 =  22<<error_bit | error;    /**< Placeholder error id */
        const uint16_t error_to_be_used_7 =  23<<error_bit | error;    /**< Placeholder error id */

        const uint16_t warning_bit = error_bit + 6 + 1;                   /**< Bit to indicate warning, error bit plus 2^6-1 error messages, bit 11 for warning and 2^5-1 warning messages.*/
        const uint16_t warning = 1 << (warning_bit - 1);                  /**< General warning id */
        const uint16_t warning_exo_run_time = 1 << warning_bit | warning; /**< Running time overflow warning id */

        const uint16_t warning_to_be_used_1 = 2 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_2 = 3 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_3 = 4 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_4 = 5 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_5 = 6 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_6 = 7 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_7 = 8 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_8 = 9 << warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_9 = 10 << warning_bit | warning;  /**< Placeholder warning id */
        const uint16_t warning_to_be_used_10 = 11 << warning_bit | warning; /**< Placeholder warning id */
        const uint16_t warning_to_be_used_11 = 12 << warning_bit | warning; /**< Placeholder warning id */
        const uint16_t warning_to_be_used_12 = 13 << warning_bit | warning; /**< Placeholder warning id */
        const uint16_t warning_to_be_used_13 = 14 << warning_bit | warning; /**< Placeholder warning id */
        const uint16_t warning_to_be_used_14 = 15 << warning_bit | warning; /**< Placeholder warning id */
    }
}

void print_status_message(uint16_t message);

#endif
