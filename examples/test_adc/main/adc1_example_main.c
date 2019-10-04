/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <adc-mestria.h>

#define GPIO_AN0 34
#define CHANNEL_AN0 6

void app_main()
{
    uint16_t valor;
    adc1_config(GPIO_AN0,CHANNEL_AN0,ADC_ATTEN_DB_11,ADC_WIDTH_BIT_12);
    while(1){
        valor = get_adc1(CHANNEL_AN0,10);   /* 10 samples of channel AN0 */
        printf("valor: %d\n",valor);
        vTaskDelay( pdMS_TO_TICKS(100));       
    }
}


