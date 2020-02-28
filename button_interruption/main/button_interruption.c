#include "gpio-01.h"
#include "timer-00.h"

#define BUTTON_COUNT          GPIO_NUM_25 // GPIO25 = IN0
#define GPIO_INPUT_PIN_COUNT  (1ULL << BUTTON_COUNT)
#define ESP_INTR_FLAG_DEFAULT 0

volatile uint16_t counter = 0;

// interrupt ISR for litres 
static void IRAM_ATTR interrupt_isr_handler(void* arg)
{
   counter++;
}

void app_main(void)
{
    // gpio configuration 
    // count   
    gpio_configuration(GPIO_PIN_INTR_POSEDGE, GPIO_MODE_INPUT, GPIO_INPUT_PIN_COUNT);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BUTTON_COUNT, interrupt_isr_handler, (void*) BUTTON_COUNT);

    bool button_count_state = 0;

    while (1)
    {
        button_count_state = gpio_digital_read(BUTTON_COUNT);
        printf("state: %d\n", button_count_state);
        printf("counter: %d\n\n", counter);

        delay(200);
    }
    
}
