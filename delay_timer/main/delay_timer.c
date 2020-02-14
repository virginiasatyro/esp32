#include <stdio.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "timer-00.h"

void app_main(void)
{
    printf("Hello world!\n");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        delay_s(1); // 1 second delay 
    }

    delay_ms(100); // 100 ms delay 
    delay(100);    // 100 ms delay 
    
    printf("Restarting now...\n");
    fflush(stdout);
    esp_restart();
}
