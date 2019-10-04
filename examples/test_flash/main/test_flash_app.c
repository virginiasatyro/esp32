#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "flash-mestria.h"


void app_main(){
    int32_t counter;
    initialize_flash();
    esp_err_t err;
    int i;
    
    err = read_int32_variables("test-storage","counter",(int32_t*) &counter);


    if(err == ESP_ERR_NVS_NOT_FOUND)
        counter = 0;

    while(1){
        for(i =0;i<10;i++){
            write_int32_variables("test-storage",(int32_t) counter,"counter");
            printf("counter: %d\n",counter);
            counter++;
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        esp_restart();
    }
}
