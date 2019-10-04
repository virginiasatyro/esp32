#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcf8574.h"
#include "hd44780.h"


#define LCD_COLS 20
#define SDA_GPIO 15
#define SCL_GPIO 4
#define I2C_ADDR 0x27

esp_err_t write_lcd_data(uint8_t data);

esp_err_t lcd_init();

esp_err_t lcd_splash(char *msg_to_print);

esp_err_t lcd_version(char *msg_to_print);

esp_err_t lcd_print(char *msg_to_print,int col,int line);

esp_err_t lcd_print_int_value(int value,char *msg_to_print_before,char *msg_to_print_after,int col,int line);
