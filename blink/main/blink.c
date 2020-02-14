#include "gpio-01.h"
#include "timer-00.h"

#define LED_BUILT_IN GPIO_NUM_2
#define GPIO_OUTPUT_PIN_SEL (1ULL << LED_BUILT_IN)

void app_main(void)
{
    gpio_configuration(GPIO_PIN_INTR_DISABLE, GPIO_MODE_OUTPUT, GPIO_OUTPUT_PIN_SEL);

    while (1)
    {
        gpio_digital_write(LED_BUILT_IN, ON);
        delay(1000);
        gpio_digital_write(LED_BUILT_IN, OFF);
        delay(1000);
    }
    
}
