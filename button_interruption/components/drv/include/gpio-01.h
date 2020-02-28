#ifndef _GPIO_01
#define _GPIO_01

#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define ON  1
#define OFF 0

esp_err_t gpio_configuration(int intr, gpio_mode_t mode, uint64_t bit_mask);

esp_err_t gpio_digital_write(gpio_num_t gpio, uint32_t level);

int gpio_digital_read(gpio_num_t gpio);

#endif // _GPIO_01