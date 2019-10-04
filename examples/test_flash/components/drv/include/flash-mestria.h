#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

void initialize_flash();

void write_int32_variables(char *storage,int32_t number_to_save,char *commit);

esp_err_t read_int32_variables(char *storage,char *commit,int32_t *number);
