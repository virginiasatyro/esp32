#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2c.h"

#define I2C_MASTER_SDA_IO   15
#define I2C_MASTER_SCL_IO   4
#define I2C_MASTER_FREQ_HZ  400000
#define READ_AND_WRITE_SIZE 7
#define TIMER_REGISTER_ADDR 0X02
#define PCF8563_ADDR        0xA2

struct rtc{
	unsigned short	year;	// 2000..2099
	unsigned char	month;	// 1..12
	unsigned char	mday;	// 1.. 31
	unsigned char	wday;	// 1..7
	unsigned char	hour;	// 0..23
	unsigned char	min;	// 0..59
	unsigned char	sec;	// 0..59
};

void rtc_initialization(int i2c_master_port);

void rtc_set_time(struct rtc time);

void rtc_get_time(struct rtc *time);
