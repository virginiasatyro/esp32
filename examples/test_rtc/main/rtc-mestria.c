#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "i2c.h"
#include "rtc-mestria.h"

int i2c_define_master_port;

void rtc_initialization(int i2c_master_port){ // init
    
    i2c_define_master_port = i2c_master_port;
    
    i2c_config_t conf;

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    esp_err_t err = i2c_master_init(conf,i2c_define_master_port);
    
    if(err == ESP_OK)
        printf("OK\n");
    else if(err == ESP_ERR_INVALID_ARG)
        printf("Invalid argument\n");
    else if(err == ESP_FAIL)
        printf("Fail\n");
    else if(ESP_ERR_INVALID_STATE)
        printf("Invalide state\n");
    else if(ESP_ERR_TIMEOUT)
        printf("Timeout\n");

}

void rtc_set_time(struct rtc time){
    unsigned char buf[7];

    if (time.sec > 59)
		time.sec = 59;
	if (time.min > 59)
		time.min = 59;
	if (time.hour > 23)
		time.hour = 23;
	if (time.mday > 31)
		time.mday = 31;
	if (time.mday == 0)
		time.mday = 1;
	if (time.wday > 6)
		time.wday = 6;
	if (time.month > 12)
		time.month = 12;
	if (time.month == 0)
		time.month = 1;

	buf[0] = (time.sec / 10 * 16 + time.sec % 10) & 0x7F;
	buf[1] = (time.min / 10 * 16 + time.min % 10) & 0x7F;
	buf[2] = (time.hour / 10 * 16 + time.hour % 10) & 0x3F;
	buf[3] = (time.mday / 10 * 16 + time.mday % 10) & 0x3F;
	buf[4] = (time.wday & 7) & 0x07;
	buf[5] = (time.month / 10 * 16 + time.month % 10) & 0x1F;
	buf[6] = (time.year - 2000) / 10 * 16 + (time.year - 2000) % 10;

    esp_err_t err;
    err =  i2c_master_write_slave(i2c_define_master_port, buf , READ_AND_WRITE_SIZE,TIMER_REGISTER_ADDR,PCF8563_ADDR);
    
    if(err == ESP_OK)
        printf("OK\n");
    else if(err == ESP_ERR_INVALID_ARG)
        printf("Invalid argument\n");
    else if(err == ESP_FAIL)
        printf("Fail\n");
    else if(ESP_ERR_INVALID_STATE)
        printf("Invalide state\n");
    else if(ESP_ERR_TIMEOUT)
        printf("Timeout\n");
    

}

void rtc_get_time(struct rtc *time){
    unsigned char buf[7];
    
    esp_err_t err;
    err = i2c_master_read_slave(i2c_define_master_port, buf , READ_AND_WRITE_SIZE,TIMER_REGISTER_ADDR,PCF8563_ADDR);
    if(err == ESP_OK)
        printf("OK\n");
    else if(err == ESP_ERR_INVALID_ARG)
        printf("Invalid argument\n");
    else if(err == ESP_FAIL)
        printf("Fail\n");
    else if(ESP_ERR_INVALID_STATE)
        printf("Invalide state\n");
    else if(ESP_ERR_TIMEOUT)
        printf("Timeout\n");

    time->sec = (buf[0] & 0x0F) + ((buf[0] >> 4) & 7) * 10;
	time->min = (buf[1] & 0x0F) + ((buf[1] >> 4) & 7) * 10;
	time->hour = (buf[2] & 0x0F) + ((buf[2] >> 4) & 3) * 10;
	time->mday = (buf[3] & 0x0F) + ((buf[3] >> 4) & 3) * 10;
	time->wday = (buf[4] & 0x07);
	time->month = (buf[5] & 0x0F) + ((buf[5] >> 4) & 1) * 10;
	time->year = 2000 + (buf[6] & 0x0F) + (buf[6] >> 4) * 10;
}
