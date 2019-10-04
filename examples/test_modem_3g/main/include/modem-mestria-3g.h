#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"
#include "gpio-mestria.h"

#define LED                             2 /* GPIO2 = LED */
#define BIT_MASK_LED                    (1ULL<<LED)
#define BUF_SIZE                        1024
#define MODEM_MSG_HEADER                0xAA        /* HEADER of modem packet */
#define TRUNCATED_MSG                   0x26        /* default value for truncated message packet */
#define RAW_MSG                         0x27        /* default value for raw message packet */
#define ACK_LENGTH                      5           /* packet ack length */
#define ACK_ERROR                       0XFF        /* error from modem ack */
#define SHORT_RAW_MSG_LENGTH            9     /* raw message length for a small packet */
#define TIME_TO_RESENT_PACKET_MS        20000
#define DEFAULT_TIMEOUT                 1000 /* 1 second */
#define UPSDA_TIMEOUT                   180000 /* 180 seconds */
#define UHTTPC_TIMEOUT                  180000  /* 180 seconds */

#define MODEM_BAUD_RATE                 115200        /* baud rate for modem application */

int compare_response(uint8_t *data,char *compare,int length_data);

void at_command(char *data);

char at_response(uint8_t *data_rcv,int timeout);

uint16_t crc16_lsb(uint8_t *pData, uint16_t length);

int ack_raw_msg_verify(uint8_t *ack);

int modem_send_short_raw_msg(uint8_t *data,uint8_t n,int gpio_relay);

esp_err_t modem_init(uart_port_t uart_num,int tx_io_num,int rx_io_num, int buf_size);

int resending_packet(uint8_t *data,uint8_t n,int gpio_relay,int err);

void printing_modem_response(int err);
