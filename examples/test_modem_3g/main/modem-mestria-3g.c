#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"
#include "gpio-mestria.h"
#include "modem-mestria-3g.h"

int uart_num_to_send = 0;

void at_command(char *data){
    //int data_length = strlen(data);
    uint8_t data_to_send[500]; 
    strcpy((char *)data_to_send,data);
    strcat((char *)data_to_send,"\r"); /* Append \r(needed to send a command to the modem) */

    uart_write_bytes(UART_NUM_2,(const char *) data_to_send,strlen((const char *)data_to_send));
}

char at_response(uint8_t *data_rcv,int timeout){
    int length = 0,i;
    length = uart_read_bytes(UART_NUM_2,data_rcv,1024,pdMS_TO_TICKS(timeout)); /* receiving data from modem */ 
    for(i = 0; i<length; i++){
        printf("%c",data_rcv[i]);
    }
    printf("\n");
    return length;
}

int compare_response(uint8_t *data,char *compare,int length_data){
    int length_compare = strlen(compare);
    int i,j;
    char str[300];


    for(i=0; i<length_data; i++){
        if(data[i] == compare[0]) /* Compare first compare string caracther with each data string position */
        {
            for(j=0;j<length_compare;j++){
                str[j] = data[i+j];     /* Assign the data word to verify if it is equal to compare string */
            }
            str[j] = '\0';              
            if(strcmp(str,compare) == 0)
                return 1;
        }
    }

    /* If there is not an equal word return 0 */
    return 0;
}

esp_err_t modem_init(uart_port_t uart_num,int tx_io_num,int rx_io_num, int buf_size){

    esp_err_t err;

    uart_num_to_send = uart_num;
    
    gpio_configuration(GPIO_INTR_DISABLE,GPIO_MODE_OUTPUT,BIT_MASK_LED); /* output configuration for an alert LED */

    /* uart configuration and uart interrupt configuration */
    err = UART_config(uart_num,tx_io_num,rx_io_num,MODEM_BAUD_RATE,UART_DATA_8_BITS,UART_PARITY_DISABLE,UART_STOP_BITS_1,UART_HW_FLOWCTRL_DISABLE,
            buf_size); 

    printf("\naguardando inicializacao do modem\n");
    vTaskDelay(pdMS_TO_TICKS(30000)); /* wait 30 seconds before receive any message */
    /* end of UART configuration */

    return err; /* return value error */
}
