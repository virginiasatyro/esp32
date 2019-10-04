#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#define NACK_VAL                    0x1
#define ACK_VAL                     0x0
#define ACK_CHECK_EN                0x1
#define ACK_CHECK_DIS               0x0
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_SDA_IO           15
#define I2C_MASTER_SCL_IO           4
#define BIT_TO_WRITE                0x00
#define BIT_TO_READ                 0x01

esp_err_t i2c_master_init(i2c_config_t conf,int i2c_master_port);

esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size,uint8_t i2c_register_addr, uint8_t device_addr);

esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size, uint8_t i2c_register_addr, uint8_t device_addr);
