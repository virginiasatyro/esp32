#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "uart-mestria.h"
#include "gpio-mestria.h"
#include "modem-task.h"
#include "modem-mestria-3g.h"

struct apn a;
struct server s;

void apn_init(struct apn *a){
    strcpy(a->apn_provider,"\"zap.vivo.com.br\"");
    strcpy(a->username,"\"vivo\"");
    strcpy(a->password,"\"vivo\"");
}

void server_init(struct server *s){
    strcpy(s->IP,"\"191.239.243.215\"");
    s->port = 8080;
    strcpy(s->url_path,"\"http://mestriaportal.brazilsouth.cloudapp.azure.com:8080/gprs\"");
}

void at_http_post(uint8_t *message,int data_length){
    char at_string[300];
    char at_string_aux[300][3];
    char at_string_aux_to_send[300];
    //char aux;
    uint8_t data[300];
    int length = 0;
    int i;
    int attempt = 0;
    at_string_aux_to_send[0] = '\0';  /* initializing the data string to send */

    /* converting values to string in hexa */
    for(i=0; i<data_length; i++){
        sprintf(at_string_aux[i],"%02x",message[i]);
        strcat(at_string_aux_to_send,at_string_aux[i]);
    }
    printf("at_string_aux_to_send: %s\n",at_string_aux_to_send);
    /* end of conversion */

    /* Creating a string to send an HTTP post */
    strcpy(at_string,"AT+UHTTPC=0,5,");
    strcat(at_string,s.url_path);
    strcat(at_string,",\"texto.txt\",");
    strcat(at_string,"\"");
    strcat(at_string,"MES-01001,0x");
    strcat(at_string,at_string_aux_to_send);
    strcat(at_string,"\"");
    strcat(at_string,",1");
    printf("%s\n",at_string);
    /* end of string created to send an HTTP post */

    /* at_command to send an HTTP post */
    at_command(at_string);   
    length = at_response(data,UHTTPC_TIMEOUT);
    vTaskDelay(pdMS_TO_TICKS(100));
    /* end of at command to HTTP post */

    /* compare response from modem */
    while(attempt < 3){
        if(compare_response(data,"0,5,1",length)){
            printf("Mensagem enviada com sucesso\n");
            attempt = 3;
        }
        else{
            attempt++;
            printf("Falha ao enviar mensagem\n");
        }
    }
    /* end of compare response */
}

void at_init(){
    /* Initializing uart connection to modem communication */
    modem_init(UART_NUM_2,TX,RX,BUF_SIZE);
    /* End of uart connection */
    
    /* Initializing structs to use server and apn information */
    apn_init(&a);
    server_init(&s);
    /* End of server and apn struct initialization */

    /* Creating variables */
    uint8_t *data = (uint8_t *) malloc(300);
    int flag_to_state_machine = 1;
    int state = INIT_STATE;
    int attempt = 0;
    int length = 0;
    char at_string[300];
    char at_string_aux[300];
    /* End of cerating variables */

    /* State machine to initializing modem with at commands */
    while(flag_to_state_machine){ 
        printf("state: %d\n",state);
        switch(state){
            case INIT_STATE:
                printf("Estado: Inicio\n");
                /* First command to communicate to modem */
                at_command("AT"); 
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                /* Verify if there is an "OK" on string response */
                if(compare_response(data,"OK",length)){
                    attempt = 0;
                    state = ECHO_REMOVE;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case ECHO_REMOVE:
                printf("Estado: Remocao do echo\n");
                at_command("ATE0");   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    attempt = 0;
                    state = ERROR_ACTIVATION;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case ERROR_ACTIVATION:
                printf("Estado: Ativacao do erro\n");
                at_command("AT+CMEE=2");
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    attempt = 0;
                    state = SIM_VERIFICATION;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case SIM_VERIFICATION:
                printf("Estado: Verificacao do cartao SIM\n");
                /* Command to verify SIM CARD */
                at_command("AT+CCID");   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"SIM not inserted",length)){
                    attempt++;
                    /* Try 3 times before break out */
                    if(attempt == 3)
                        state = BREAK;
                }
                else{
                    attempt = 0;
                    state = APN_CONFIGURATION;
                }
                break;
            case APN_CONFIGURATION:
                printf("Estado: Configuracao inicial da APN\n");
                strcpy(at_string,"AT+UPSD=0,1,");
                strcat(at_string,a.apn_provider);
                at_command(at_string);   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = APN_USERNAME;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case  APN_USERNAME:
                printf("Estado: Configuracao do username da APN\n");
                strcpy(at_string,"AT+UPSD=0,2,");
                strcat(at_string,a.username);
                at_command(at_string);   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = APN_PASSWORD;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case APN_PASSWORD:
                printf("Estado: Configuracao da senha da APN\n");
                strcpy(at_string,"AT+UPSD=0,3,");
                strcat(at_string,a.password);
                at_command(at_string);   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = CONECTION_ACTIVATION;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case CONECTION_ACTIVATION:
                printf("Estado: Estabelecimento de conexão\n");
                at_command("AT+UPSDA=0,3");   
                length = at_response(data,UPSDA_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = APN_IP_ADDRESS;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case APN_IP_ADDRESS:
                printf("Estado: Obtencao de endereco IP APN\n");
                at_command("AT+UPSND=0,0");   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = HTTP_REFRESH;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case HTTP_REFRESH:
                printf("Estado: Atualizacao da conexao HTTP\n");
                at_command("AT+UHTTP=0");   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = SERVER_IP_ADDRESS;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case SERVER_IP_ADDRESS:
                printf("Estado: Configuracao do endereco IP do servidor\n");
                strcpy(at_string,"AT+UHTTP=0,0,");
                strcat(at_string,s.IP);
                at_command(at_string);   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = SERVER_PORT;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case SERVER_PORT:
                printf("Estado: Configuracao da Porta do servidor\n");
                strcpy(at_string,"AT+UHTTP=0,5,");
                itoa(s.port,at_string_aux,10);
                strcat(at_string,at_string_aux);
                at_command(at_string);   
                length = at_response(data,DEFAULT_TIMEOUT);
                vTaskDelay(pdMS_TO_TICKS(100));
                if(compare_response(data,"OK",length)){
                    state = SUCSSESSFUL;
                    attempt = 0;
                }
                else{
                    attempt++;
                    if(attempt == 3)
                        state = BREAK;
                }
                break;
            case SUCSSESSFUL:
                printf("Estado: Sucesso\n");
                printf("Estado: Configuracao realizada com sucesso!\n");
                state = BREAK;
                break;
            case BREAK:
                flag_to_state_machine = 0;
                break;
            default:
                flag_to_state_machine = 0;
                break;
        }
    }
    printf("Fim da state machine\n");
    /* End of state machine initialization */
}
