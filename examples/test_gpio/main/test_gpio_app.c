#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "gpio-mestria.h"

#define OUTPUT_1	            23 /* GPIO23 = OUT0 */
#define OUTPUT_2                21 /* GPIO21 = OUT1 */
#define LED_1                   2 /* GPIO2 = LED */
#define GPIO_OUTPUT_PIN_SEL     ((1ULL<<OUTPUT_1) | (1ULL<<LED_1) | (1ULL<<OUTPUT_2))

#define BUTTON_1	            25 /* GPIO25 = IN0 */
#define BUTTON_2                26 /* GPIO26 = IN1 */
#define ESP_INTR_FLAG_DEFAULT   0
#define GPIO_INPUT_PIN_1    (1ULL<<BUTTON_1)
#define GPIO_INPUT_PIN_2    (1ULL<<BUTTON_2)

volatile int counter=0;

static void IRAM_ATTR interrupt_isr_handler(void* arg) /* interrupt ISR for litres */
{
   counter++;
}


void app_main(){
    gpio_configuration(GPIO_PIN_INTR_DISABLE,GPIO_MODE_OUTPUT,GPIO_OUTPUT_PIN_SEL);
    gpio_configuration(GPIO_PIN_INTR_DISABLE,GPIO_MODE_INPUT,GPIO_INPUT_PIN_2);
    gpio_configuration(GPIO_PIN_INTR_POSEDGE,GPIO_MODE_INPUT,GPIO_INPUT_PIN_1);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BUTTON_1, interrupt_isr_handler, (void*) BUTTON_1);


    int a = 0;
    int state;
    while(1){

        gpio_digital_write(LED_1, ON);
        gpio_digital_write(OUTPUT_1, counter%2);
        state = gpio_digital_read(BUTTON_2);
        printf("state: %d\n",state);
        printf("counter: %d\n",counter);
        printf("\n");
        a++;
        vTaskDelay(pdMS_TO_TICKS(500)); // delay
        gpio_digital_write(LED_1, OFF);
        vTaskDelay(pdMS_TO_TICKS(500)); // delay

        // ACENDE E APAGA LED DE ACORDO COM O ESTADO DOS BOTÕES
        /*int state_IN0 = gpio_digital_read(BUTTON_1);
        int state_IN1 = gpio_digital_read(BUTTON_2);

        if(state_IN0 == 1){
          gpio_digital_write(LED_1, ON);
        }
        if(state_IN1 == 1){
          gpio_digital_write(LED_1, OFF);
        }

        printf("state_IN0: %d    state_IN1: %d\n", state_IN0, state_IN1);
        vTaskDelay(pdMS_TO_TICKS(300)); // delay*/
    }
}
