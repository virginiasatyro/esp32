#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"
#include "gpio-mestria.h"
#include "modem-mestria-3g.h"

#define INIT_STATE                      0
#define ECHO_REMOVE                     1          /* packet send successfull */
#define ERROR_ACTIVATION                2          /* packet length bigger than maximum length */
#define SIM_VERIFICATION                3          /* wrong CRC from modem */
#define APN_CONFIGURATION               4          /* incorret header message from modem */
#define APN_USERNAME                    5          /* wrong ack length from modem */          
#define APN_PASSWORD                    6          /* error messa from modem */
#define CONECTION_ACTIVATION            7          /* not identfied error */
#define APN_IP_ADDRESS                  8
#define HTTP_REFRESH                    9
#define SERVER_IP_ADDRESS               10
#define SERVER_PORT                     11
#define DATA_SEND_TO_SERVER             12
#define SUCSSESSFUL                     13
#define BREAK                           14

#define TX                      17                  /* pin 17 for TX2 */
#define RX                      16                  /* pin 16 RX2 */

struct apn{
    char apn_provider[128];
    char username[128];
    char password[128];
};

struct server{
    char IP[20];
    int port;
    char url_path[128];
};

void at_init();

void at_http_post(uint8_t *message,int data_length);
