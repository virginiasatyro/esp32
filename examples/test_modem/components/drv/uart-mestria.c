#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"

esp_err_t UART_config(uart_port_t uart_num,int tx_io_num,int rx_io_num,int baud_rate,uart_word_length_t data_bits,uart_parity_t parity,                                                                                                                                 uart_stop_bits_t stop_bits,uart_hw_flowcontrol_t flow_ctrl, int buf_size)
{
    esp_err_t return_value;
    
    uart_config_t uart_config_0 = {
        .baud_rate = baud_rate,
        .data_bits = data_bits,
        .parity    = parity,
        .stop_bits = stop_bits,
        .flow_ctrl = flow_ctrl
    };
    return_value = uart_param_config(uart_num, &uart_config_0);
    if(return_value != ESP_OK)
        return return_value;
    return_value = uart_set_pin(uart_num, tx_io_num, rx_io_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if(return_value != ESP_OK)
        return return_value;
    return_value = uart_driver_install(uart_num, buf_size * 2, 0, 0, NULL, 0);
    return return_value;
}


esp_err_t UART_config_rx_intr(uart_port_t uart_num, void (*fn)(void*)){
    esp_err_t return_value;
    uart_isr_handle_t handle_console;
    
    uart_isr_free(uart_num);       
    return_value = uart_enable_rx_intr(uart_num);
    uart_isr_register(UART_NUM_0,fn, NULL, ESP_INTR_FLAG_IRAM, &handle_console);

    return return_value;
}
