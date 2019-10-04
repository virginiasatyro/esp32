#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc-mestria.h"

#define I2C_PORT    I2C_NUM_0

void app_main(){
    struct rtc time;
    
    rtc_initialization(I2C_PORT);
    
    time.sec = 0;
    time.min = 38;
    time.hour = 13;
    time.mday = 3;
    time.wday = 6;
    time.month = 10;
    time.year = 2019;

    //rtc_set_time(time);

    rtc_get_time(&time);

    printf("%02d/",time.mday);
    printf("%02d/",time.month);
    printf("%04d ",time.year);

    printf("%02d:",time.hour);
    printf("%02d:",time.min);
    printf("%02d \n",time.sec);
}
