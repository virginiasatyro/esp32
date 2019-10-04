#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "flash-mestria.h"

void initialize_flash(){
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}


void write_int32_variables(char *storage,int32_t number_to_save,char *commit){
    esp_err_t err = nvs_flash_init();
    nvs_handle my_handle;
    err = nvs_open(storage, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Write
        err = nvs_set_i32(my_handle, commit, number_to_save);
        printf((err != ESP_OK) ? "Failed!\n" : "");

        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "");

        // Close
        nvs_close(my_handle);
    }

}


esp_err_t read_int32_variables(char *storage,char *commit,int32_t *number){
    esp_err_t err = nvs_flash_init();
    nvs_handle my_handle;
    err = nvs_open(storage, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Read
        err = nvs_get_i32(my_handle, commit, number);
    }
    // Close
    nvs_close(my_handle);

    return err;
}
