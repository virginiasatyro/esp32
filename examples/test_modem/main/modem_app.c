#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "modem-mestria.h"
#include "gpio-mestria.h"


#define GPIO_RELAY              23  
#define BIT_MASK_RELAY          (1ULL<<GPIO_RELAY)
#define TX                      17                  /* pin 17 for TX2 */
#define RX                      16                  /* pin 16 RX2 */

void app_main(){
    modem_init(UART_NUM_2,TX,RX,BIT_MASK_RELAY,BUF_SIZE);
    uint8_t msg[9];
    int err;

    msg[0] = 0x10;
    msg[1] = 0x22;
    msg[2] = 0x33;
    msg[3] = 0x44;
    msg[4] = 0x55;
    msg[5] = 0x66;
    msg[6] = 0x77;
    msg[7] = 0x88;
    msg[8] = 0x99;

    vTaskDelay(pdMS_TO_TICKS(5000));

    while(1){
        err = modem_send_short_raw_msg(msg,9,GPIO_RELAY);
        if(err != PACKET_SENT_SUCCESSFULLY)
            err = resending_packet(msg,9,GPIO_RELAY,err);
        printing_modem_response(err);
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
