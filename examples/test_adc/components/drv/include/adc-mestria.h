#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/adc.h"


void adc1_config(uint16_t gpio_pin,uint16_t channel,uint16_t attenuation,uint16_t width);

uint16_t get_adc1(uint16_t channel,int samples);
