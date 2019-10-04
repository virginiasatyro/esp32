#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"

#define BUF_SIZE                (1024)  /* length of send and receive buffer */
#define TX0                     UART_PIN_NO_CHANGE  /* default pin for TX0 */
#define RX0                     UART_PIN_NO_CHANGE  /* default pin for RX0 */
#define BAUD_RATE               115200

int uart_flag = 0,UART_packet_length = 0;
uint8_t rxbuf[1024];
uint16_t rx_fifo_len, status;

static void IRAM_ATTR uart_intr_handle(void *arg)/* interrupt for UART application */
{
    int j = 0;
    status = UART0.int_st.val; // read UART interrupt Status

    rx_fifo_len = UART0.status.rxfifo_cnt; // read number of bytes in UART buffer
    UART_packet_length = rx_fifo_len;
    while(rx_fifo_len){
        rxbuf[j++] = UART0.fifo.rw_byte; // read all bytes
        rx_fifo_len--;
    }
    uart_clear_intr_status(UART_NUM_0, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
    uart_flag = 1;
}


void app_main(){

    UART_config(UART_NUM_0,TX0,RX0,BAUD_RATE,UART_DATA_8_BITS,UART_PARITY_DISABLE,UART_STOP_BITS_1,UART_HW_FLOWCTRL_DISABLE,2*(BUF_SIZE)); /* UART configuration */
    UART_config_rx_intr(UART_NUM_0,uart_intr_handle);

    while(1){
        if(uart_flag == 1){
            uart_flag = 0;
            uart_write_bytes(UART_NUM_0, (const char *) rxbuf, UART_packet_length);
            uart_write_bytes(UART_NUM_0, "\n", sizeof(char));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
