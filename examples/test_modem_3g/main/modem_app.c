#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "modem-mestria-3g.h"
#include "gpio-mestria.h"
#include "modem-task.h"

void app_main(){
    at_init();

    while(1){
        uint8_t data[8];
        data[0] = 0x02;
        data[1] = 0x01;
        data[2] = 0x00;
        data[3] = 0x00;
        data[4] = 0xa4;
        data[5] = 0x00;
        data[6] = 0x2e;
        data[7] = 0x01;
        at_http_post(data,8);
        vTaskDelay(pdMS_TO_TICKS(300000));
    }
}
