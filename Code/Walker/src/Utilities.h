
#ifndef Utilities_h
#define Utilities_h

#include "ParseIni.h"
#include "Arduino.h"
#include <stdint.h>
#include <utility>  //std::pair
#include <queue>    //std::queue


namespace utils
{
    
    bool get_is_left(config_defs::joint_id id);
    
    
    bool get_is_left(uint8_t id);

    
    uint8_t get_joint_type(config_defs::joint_id id);
    
   
    uint8_t get_joint_type(uint8_t id);
    
    
    bool schmitt_trigger(float value, bool is_high, float lower_threshold, float upper_threshold);
    
   
    int rate_limit(int setpoint, int last_value, int* last_time, int rate_per_ms);
    
    uint8_t update_bit(uint8_t original, bool val, uint8_t loc);
    
    
    uint16_t update_bit(uint16_t original, bool val, uint8_t loc);
    
 
    bool get_bit(uint8_t original, uint8_t loc);
    

    bool get_bit(uint16_t original, uint8_t loc);
    
    float degrees_to_radians(float);

    float radians_to_degrees(float);

    String remove_all_chars(String str, char rmv);
    String remove_all_chars(char* arr, int len, char rmv);

 
    int get_char_length(int ofInt);
    

    template <typename T>
    int elements_are_equal(T arr1, T arr2, int length)
    {
        for (int i=0; i<length; i++)
        {
            if(arr1[i] != arr2[i])
            {
                return 0;
            }
        }
        return 1;
    };


    template <typename T>
    void set_elements_equal(T arr1, T arr2, int length)
    {
        for (int i=0; i<length; i++)
        {
            arr1[i] = arr2[i];
        }
    };
    

    class SpeedCheck
    {
        public:
            SpeedCheck(int pin);
            
            /**
             * @brief toggles the pin attached to the object
             */
            void toggle();
            
        private:
            int _pin;       /**< Pin to use for the check */
            bool _state;    /**< State of the pin */
    };
    

    bool is_little_endian();
    

    void float_to_uint8(float num_to_convert, uint8_t *converted_bytes);
    

    void uint8_to_float(uint8_t *bytes_to_convert, float *converted_float);
    

    void float_to_short_fixed_point_bytes(float num_to_convert, uint8_t *converted_bytes, uint8_t factor);
    

    void short_fixed_point_bytes_to_float(uint8_t *bytes_to_convert, float *converted_val, uint8_t factor);
    

    float ewma(float new_value, float filter_value, float alpha);


    void spin_on_error_with(String message);


    bool is_close_to(float val1, float val2, float tolerance);


    std::pair<float, float> online_std_dev(std::queue<float> set);

    bool is_outside_range(float val, float min, float max);
}

#endif