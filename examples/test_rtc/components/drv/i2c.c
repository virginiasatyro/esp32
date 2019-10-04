#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "i2c.h"

static esp_err_t last_i2c_err = ESP_OK; 

esp_err_t i2c_master_init(i2c_config_t conf,int i2c_master_port)
{
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
            I2C_MASTER_RX_BUF_DISABLE,
            I2C_MASTER_TX_BUF_DISABLE, 0);
}

esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size,uint8_t i2c_register_addr, uint8_t device_addr)
{
    last_i2c_err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, device_addr | BIT_TO_WRITE, true);
    i2c_master_write_byte(cmd, i2c_register_addr, true);
    i2c_master_write(cmd, data_wr, size, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    last_i2c_err = ret;
    printf("last_i2c_err %d\n", last_i2c_err);
    return ret;

}

esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size, uint8_t i2c_register_addr, uint8_t device_addr)
{
    last_i2c_err = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, device_addr | BIT_TO_WRITE, true);
    i2c_master_write_byte(cmd, i2c_register_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, device_addr | BIT_TO_READ, true);
    i2c_master_read(cmd, data_rd, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    last_i2c_err = ret;
    return ret;
}
