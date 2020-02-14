#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "gpio-01.h"

/**
 * @brief : function to configure gpio
 * @param intr : interruption
 * @param mode : GPIO_MODE_OUTPUT, GPIO_MODE_INPUT
 * @param bit_mask: 
 * @return esp_err_t: ESP_OK success or ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t gpio_configuration(int intr, gpio_mode_t mode, uint64_t bit_mask)
{
    esp_err_t ret; // return value

    gpio_config_t io_conf; // pointer to GPIO configure struct

    if(intr == GPIO_PIN_INTR_POSEDGE){
        io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    }else{
        if(intr == GPIO_PIN_INTR_NEGEDGE)
            io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
        else
            io_conf.intr_type = intr;
    }  

    io_conf.mode = mode;  // GPIO_MODE_OUTPUT
    io_conf.pin_bit_mask = bit_mask;  
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // disable pulldown
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; // disable pullup

    // GPIO common configuration - Configure GPIO’s Mode, pull-up, PullDown, IntrType
    // Return : ESP_OK success or ESP_ERR_INVALID_ARG Parameter error
    // Parameters : pGPIOConfig: Pointer to GPIO configure struct
    ret = gpio_config(&io_conf);
    
    return ret;
}

/**
 * @brief : function to write on digital pin 
 * @param gpio_num : If you want to set the output level of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16)
 * @param level: 0 or 1 - ON ou OFF 
 * @return esp_err_t: ESP_OK Success or ESP_ERR_INVALID_ARG GPIO number error
 */
esp_err_t gpio_digital_write(gpio_num_t gpio_num, uint32_t level)
{
    esp_err_t ret;
    
    if(level == ON)
        // GPIO set output level 
        // Return: ESP_OK Success or ESP_ERR_INVALID_ARG GPIO number error
        // Parameters : gpio_num: GPIO number. If you want to set the output level of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16); level: Output level. 0: low ; 1: high
        ret = gpio_set_level(gpio_num, 1);
    else{
        if(level == OFF)
            ret = gpio_set_level(gpio_num, 0);
        else
            return -1;
    }

    return ret;
}

/**
 * @brief : function to read a digital pin
 * @param gpio_num : If you want to set the output level of e.g. GPIO16, gpio_num should be GPIO_NUM_16 (16) 
 * @return int: 
 */
int gpio_digital_read(gpio_num_t gpio_num)
{
    int ret;

    // GPIO get input level 
    // Return : 0 the GPIO input level is 0 - 1 the GPIO input level is 1
    // Parameters : gpio_num: GPIO number. If you want to get the logic level of e.g. pin GPIO16, gpio_num should be GPIO_NUM_16 (16);
    ret = gpio_get_level(gpio_num);
    // if(ret  == 0)
    //     ret = 1;
    // else{
    //     if(ret == 1)
    //         ret = 0;
    //     else
    //         return -1;
    // }

    return ret;
}