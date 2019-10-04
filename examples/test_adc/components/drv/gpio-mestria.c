#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "gpio-mestria.h"

esp_err_t gpio_configuration(int intr,gpio_mode_t mode,uint64_t bit_mask){
    gpio_config_t io_conf; 	
    esp_err_t return_value;
    
    if(intr == GPIO_PIN_INTR_POSEDGE)
        io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    else{
        if(intr == GPIO_PIN_INTR_NEGEDGE)
            io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
        else
            io_conf.intr_type = intr;
    }  
    io_conf.mode = mode;  
    io_conf.pin_bit_mask = bit_mask;  
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; 
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; 
    return_value = gpio_config(&io_conf);
    return return_value;
}

esp_err_t gpio_digital_write(gpio_num_t gpio,uint32_t level){
    esp_err_t return_value;
    
    if(level == ON)
        return_value = gpio_set_level(gpio, 1);
    else{
        if(level == OFF)
            return_value = gpio_set_level(gpio, 0);
        else
            return -1;
    }
    return return_value;
}

int gpio_digital_read(gpio_num_t gpio){
    int return_value;

    return_value = gpio_get_level(gpio);
    if(return_value  == 0)
        return_value = 1;
    else{
        if(return_value == 1)
            return_value = 0;
        else
            return -1;
    }
    return return_value;
}
