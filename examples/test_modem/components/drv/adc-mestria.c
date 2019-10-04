#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include <adc-mestria.h>

void adc1_config(uint16_t gpio_pin,uint16_t channel,uint16_t attenuation,uint16_t width){
    uint64_t adc_pin_assignment = (1ULL<<gpio_pin);

    gpio_config_t io_conf; /* Create a descriptor variable to drive GPIO */

    /* set up the descriptor inputs */
    io_conf.intr_type = GPIO_INTR_DISABLE; /* extern interrupt enabled for positive edge and negative edge */ 
    io_conf.mode = GPIO_MODE_INPUT; /* Set up as a pin input */ 
    io_conf.pin_bit_mask = adc_pin_assignment; /* assign pins to utilize as interrupt on drive */ 
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; /* Disable pulldown */
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; /* Disabe pullup */
    gpio_config(&io_conf); /* Setup GPIOs */ 

    adc1_config_width(width);
    adc1_config_channel_atten(channel,attenuation);
}

uint16_t get_adc1(uint16_t channel){
    uint16_t value = 0;
    value = adc1_get_raw(channel);
    return value;
}
