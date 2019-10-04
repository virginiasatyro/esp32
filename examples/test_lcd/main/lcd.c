#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hd44780.h"
#include "pcf8574.h"
#include "lcd.h"

static i2c_dev_t pcf8574;
static const uint8_t char_data[] = {
    0x04, 0x0e, 0x0e, 0x0e, 0x1f, 0x00, 0x04, 0x00,
    0x1f, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x1f, 0x00
};

esp_err_t write_lcd_data(uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

hd44780_t lcd = {
    .write_cb = write_lcd_data, // use callback to send data to LCD by I2C GPIO expander
    .font = HD44780_FONT_5X8,
    .lines = 4,
    .pins = {
        .rs = 0,
        .e  = 2,
        .d4 = 4,
        .d5 = 5,
        .d6 = 6,
        .d7 = 7,
        .bl = 3
    }
};

esp_err_t lcd_init()
{
    esp_err_t err;

    err = i2cdev_init();

    //Init i2cdev lib
    if(err != ESP_OK)
        return err;

    err = pcf8574_init_desc(&pcf8574, 0, I2C_ADDR, SDA_GPIO, SCL_GPIO);
    // Init I2C device descriptor
    if(err!= ESP_OK)
        return err;
        
    err = hd44780_init(&lcd);
    if(err != ESP_OK)
        return err;

    hd44780_switch_backlight(&lcd, true);

    hd44780_upload_character(&lcd, 0, char_data);
    hd44780_upload_character(&lcd, 1, char_data + 8);

    return err;
}

esp_err_t lcd_splash(char *msg_to_print){
    int length,offset;
    esp_err_t err;
    
    length = strlen(msg_to_print);
    offset = (int) (LCD_COLS-length)/2;

    err = hd44780_gotoxy(&lcd, 6, 0);
    if(err != ESP_OK)
        return err;

    err = hd44780_puts(&lcd, "Mestria");
    if(err != ESP_OK)
        return err;

    err = hd44780_gotoxy(&lcd, offset, 1);
    if(err != ESP_OK)
        return err;

    err = hd44780_puts(&lcd, msg_to_print);
    if(err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(2000));

    return err;
}

esp_err_t lcd_version(char *msg_to_print){
    int length,offset;
    esp_err_t err;

    length = strlen(msg_to_print);
    offset = (int) (LCD_COLS-length)/2;

    err = hd44780_gotoxy(&lcd, 6, 0);
    if(err != ESP_OK)
        return err;

    err = hd44780_puts(&lcd, "Mestria");
    if(err != ESP_OK)
        return err;

    err = hd44780_gotoxy(&lcd, offset, 1);
    if(err != ESP_OK)
        return err;

    err = hd44780_puts(&lcd, msg_to_print);
    if(err != ESP_OK)
        return err;

    vTaskDelay(pdMS_TO_TICKS(2000));
    err = hd44780_clear(&lcd);
    if(err != ESP_OK)
        return err;

    return err;
}

esp_err_t lcd_print(char *msg_to_print,int col,int line){
    esp_err_t err;

    err = hd44780_gotoxy(&lcd, line, col);
    if(err != ESP_OK)
        return err;
    hd44780_puts(&lcd, msg_to_print);
    if(err != ESP_OK)
        return err;

    return err;
}

esp_err_t lcd_print_int_value(int value,char *msg_to_print_before,char *msg_to_print_after,int col,int line){
    char valueToPrint[20];
    esp_err_t err;

    itoa(value,valueToPrint,10);

    err = hd44780_gotoxy(&lcd, col, line);
    if(err != ESP_OK)
        return err;

    err = hd44780_puts(&lcd, msg_to_print_before);
    if(err != ESP_OK)
        return err;

    err = hd44780_puts(&lcd, valueToPrint);
    if(err != ESP_OK)
        return err;
    err = hd44780_puts(&lcd, msg_to_print_after);
    if(err != ESP_OK)
        return err;
    
    return err;
}
