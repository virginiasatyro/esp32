#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"


esp_err_t UART_config(uart_port_t uart_num,int tx_io_num,int rx_io_num,int baud_rate,uart_word_length_t data_bits,uart_parity_t parity,                                                                                                                                 uart_stop_bits_t stop_bits,uart_hw_flowcontrol_t flow_ctrl, int buf_size);

esp_err_t UART_config_rx_intr(uart_port_t uart_num,void (*fn)(void*));
