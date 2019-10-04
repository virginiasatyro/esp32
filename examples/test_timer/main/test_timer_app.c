#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "timer-mestria.h"

#define TIMER_DIVIDER           16 /* Hardware timer clock divider */
#define TIMER_SCALE             (TIMER_BASE_CLK / TIMER_DIVIDER)

void app_main()
{
    double time_sec,time_min,time_hour;
    esp_err_t verification;


    verification = timer_config(TIMER_DIVIDER,TIMER_COUNT_UP,TIMER_PAUSE,TIMER_ALARM_DIS,TIMER_AUTORELOAD_DIS,TIMER_INTR_MAX,TIMER_GROUP_0,TIMER_0);
    if(verification != ESP_OK){
        printf("Erro ao configurar TIMER\n");
    }
    else{
        verification = timer_start_count(TIMER_GROUP_0,TIMER_0,0x00000000000);
        if(verification != ESP_OK){
            printf("Erro ao iniciar o TIMER\n");
        }
        else{
            while(1){
                timer_get_counter_time_sec(TIMER_GROUP_0,TIMER_0,&time_sec);
                timer_get_counter_min(TIMER_GROUP_0,TIMER_0,&time_min);
                timer_get_counter_hour(TIMER_GROUP_0,TIMER_0,&time_hour);
                printf("time_sec: %f\n",time_sec);
                printf("time_min: %f\n",time_min);
                printf("time_hour: %f\n",time_hour);
                printf("\n");
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}
