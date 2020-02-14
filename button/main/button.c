#include "gpio-01.h"
#include "timer-00.h"

#define BUTTON_RESTART     GPIO_NUM_25 // GPIO25 = IN0
#define GPIO_INPUT_PIN_25  (1ULL << BUTTON_RESTART)

void app_main(void)
{
    // gpio configuration 
    // restart   
    gpio_configuration(GPIO_PIN_INTR_DISABLE, GPIO_MODE_INPUT, GPIO_INPUT_PIN_25);
    
    uint16_t state_to_restart = 0;

    while (1)
    {
        state_to_restart = gpio_digital_read(BUTTON_RESTART);
        if(state_to_restart){
            fflush(stdout);
            printf("Rebooting...");
            delay(300);
            esp_restart();
        }
        delay(100);
    }
    
}
