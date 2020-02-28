#include <stdio.h>
#include <stdlib.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "timer-00.h"

/**
 * @brief : function to simplify delay sintaxe. Delay in miliseconds;
 * @param delay_ms : delay value in miliseconds;
 * @return : void 
 */
void delay(__uint64_t delay_ms)
{
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}

/**
 * @brief : function to simplify delay sintaxe. Delay in miliseconds;
 * @param delay_ms : delay value in miliseconds;
 * @return : void 
 */
void delay_ms(__uint64_t delay_ms)
{
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}

/**
 * @brief : function to simplify delay sintaxe. Delay in seconds;
 * @param delay_s : delay value in seconds;
 * @return : void 
 */
void delay_s(__uint64_t delay_s)
{
    vTaskDelay(1000 * delay_s / portTICK_PERIOD_MS);
}