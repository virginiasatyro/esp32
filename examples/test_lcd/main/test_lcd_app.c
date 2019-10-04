#include <stdio.h>
#include <stdlib.h>
#include <driver/gpio.h>
#include "lcd.h"

void app_main(){
    int counter = 0;
    esp_err_t err;
    err = lcd_init();
    if(err == ESP_OK){
        
        lcd_splash("SmartH2O");
        lcd_version("version 1.0.0");

        while(1){
            err = lcd_print_int_value(counter,"Counter: "," ",0,0);
            if(err == ESP_OK){
                counter++;
                if(counter == 100)
                    counter = 0;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else
                printf("erro ao printar valores na tela\n");
        }
    }
    else
        printf("Erro ao inicializar LCD \n");
}
