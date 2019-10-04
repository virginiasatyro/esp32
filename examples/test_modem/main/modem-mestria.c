#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"
#include "gpio-mestria.h"
#include "modem-mestria.h"

esp_err_t modem_init(uart_port_t uart_num,int tx_io_num,int rx_io_num,uint64_t bit_mask, int buf_size){

    esp_err_t err;
    
    gpio_configuration(GPIO_INTR_DISABLE,GPIO_MODE_OUTPUT,bit_mask); /* output configuration for modem sincronization */
    gpio_configuration(GPIO_INTR_DISABLE,GPIO_MODE_OUTPUT,BIT_MASK_LED); /* output configuration for an alert LED */
    err = UART_config(uart_num,tx_io_num,rx_io_num,MODEM_BAUD_RATE,UART_DATA_8_BITS,UART_PARITY_DISABLE,UART_STOP_BITS_1,UART_HW_FLOWCTRL_DISABLE,buf_size); /* modem
                                                                                                                                                            configuration*/
    
    return err; /* return value error */
}


/* crc to message verification */
uint16_t crc16_lsb(uint8_t *pData, uint16_t length)
{
	uint8_t i;
	uint16_t data, crc;
	crc = 0xFFFF;
	if (length == 0) 
		return 0;
	do
	{
		data = (uint16_t)0x00FF & *pData++;
		crc = crc ^ data;
		for (i = 8; i > 0; i--)
		{
			if (crc & 0x0001)
				crc = (crc >> 1) ^ 0x8408;
			else
				crc >>= 1;
		}
	}
	while (--length);
	crc = ~crc;
	return (crc);
}

int ack_raw_msg_verify(uint8_t *ack){ /* function that verify modem ack */
    uint16_t crc;
    uint8_t *test_ack = (uint8_t *) malloc(ACK_LENGTH-2);

    for(int i = 0 ; i < ACK_LENGTH-2; i++){
        test_ack[i] = ack[i];
    }

    crc = crc16_lsb(test_ack,ACK_LENGTH-2);

    if(ack[0] == MODEM_MSG_HEADER){
        if(ack[1] == ACK_LENGTH){
            if(ack[2] == RAW_MSG){
                if(((crc&0x00FF) == (ack[ACK_LENGTH-2])) && (crc >> 8 == ack[ACK_LENGTH-1])){
                    return PACKET_SENT_SUCCESSFULLY;
                }
                else{
                    return WRONG_CRC_RETURN;
                }
            }
            else{
                if(ack[2] == ACK_ERROR){
                    return ERROR_MESSAGE;
                }
                else{
                   return ERROR_NOT_IDENTIFIED;
                }
            }
        }
        else{
            return WRONG_ACK_LENGTH;
        }
    }
    else{
        return INCORRECT_HEADER_MESSAGE;
    }

    return ERROR_NOT_IDENTIFIED;
}

int modem_send_short_raw_msg(uint8_t *data,uint8_t n,int gpio_relay){ /* function execute a short packet sending(can send a maximum of 9 on payload field) */
    
    int length = SHORT_RAW_MSG_LENGTH + 5;      /* message has to be 14 positions */ 
    if(n > SHORT_RAW_MSG_LENGTH)                /* if payload greather than SHORT_RAW_MESSAGE_LENGTH(9) packet not send */
        return PACKET_TO_LONG;


    uint8_t rest; 
    uint16_t crc;
    uint8_t *data_rcv = (uint8_t *) malloc(BUF_SIZE);
    uint8_t *packet = (uint8_t *) malloc(SHORT_RAW_MSG_LENGTH+5);
    int i = 0,raw_message_response;

    rest = SHORT_RAW_MSG_LENGTH - n;    /* size filled with zeros */

    packet[0] = MODEM_MSG_HEADER;  /* HEADER of modem packet */
    packet[1] = length;    /* packet length */
    packet[2] = RAW_MSG;     /* raw message identifier */
    for(i = 0; i < n; i++){
        packet[i+3] = data[i];  /* putting each position of data inside packet */
    }

    for(i = 0; i < rest; i++){
        packet[i+3+n] = 0;      /* filling with zeros */
    }

    crc = crc16_lsb(packet,length-2); /* building crc */
    packet[length-2] = (uint8_t) crc&0x00FF; /* less significant crc byte */
    packet[length-1] = (uint8_t) (crc >> 8); /* most significant crc byte */
    
    gpio_digital_write(LED,ON);             /* set LED HIGH to indicate sending task */ 
    gpio_digital_write(gpio_relay,ON);      /* set mode ON on relay to modem sincronization */
    vTaskDelay(pdMS_TO_TICKS(13));           /* waits 13(10milliseconds to turn on the relay and 3 millisseconds for sincronization  milliseconds for sincronization */

    uart_write_bytes(UART_NUM_2, (const char *) packet, length);/* send the packet to modem */
    length = uart_read_bytes(UART_NUM_2, data_rcv, ACK_LENGTH, 100);/* receive ACK or NACK from modem */

    gpio_digital_write(LED,OFF);         
    gpio_digital_write(gpio_relay,OFF);
    raw_message_response = ack_raw_msg_verify(data_rcv);/* verify if the ack is correct */

    free(data_rcv); /* freeing memory */
    free(packet);
    return raw_message_response;
}

int resending_packet(uint8_t *data,uint8_t n,int gpio_relay,int err){
    int i = 0;
    printf("resending packet...\n");
    vTaskDelay(pdMS_TO_TICKS(TIME_TO_RESENT_PACKET_MS));

    while(err != PACKET_SENT_SUCCESSFULLY && i < 5){
        err = modem_send_short_raw_msg(data,n,gpio_relay);
        i++;
        if(err != PACKET_SENT_SUCCESSFULLY){
            printf("resending packet...\n");
            vTaskDelay(pdMS_TO_TICKS(TIME_TO_RESENT_PACKET_MS));
        }
    }

    return err;
}

void printing_modem_response(int err){
    switch(err){
            case PACKET_SENT_SUCCESSFULLY:
                printf("message sent successfuly\n");
                break;
            case PACKET_TO_LONG:
                printf("packet to long to send\n");
                break;
            case WRONG_CRC_RETURN:
                printf("wrong crc from modem\n");
                break;
            case INCORRECT_HEADER_MESSAGE:
                printf("incorrect header from modem\n");
                break;
            case WRONG_ACK_LENGTH:
                printf("wrong ack length from modem\n");
                break;
            case ERROR_MESSAGE:
                printf("error message from modem\n");
                break;
            case ERROR_NOT_IDENTIFIED:
                printf("error not identified\n");
                break;
            default:
                printf("response not identified\n");
                break;
        }
}
