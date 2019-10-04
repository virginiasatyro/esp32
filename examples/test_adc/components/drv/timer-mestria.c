#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "timer-mestria.h"


esp_err_t timer_config(uint32_t timer_divider,timer_count_dir_t counter_dir,bool en, bool alarm_en,bool auto_reload,timer_intr_mode_t intr_type,timer_group_t timer_group,timer_idx_t timer_idx){

    esp_err_t return_value;

    timer_config_t config;
    config.divider = timer_divider;
    config.counter_dir = counter_dir;
    config.counter_en = en;
    config.alarm_en = alarm_en;
    config.auto_reload = auto_reload;
    config.intr_type = intr_type;
    return_value = timer_init(timer_group, timer_idx, &config);
    return return_value;
}

esp_err_t timer_start_count(timer_group_t timer_group,timer_idx_t timer_idx,uint64_t load_value){
    esp_err_t return_value;

    timer_set_counter_value(timer_group, timer_idx, load_value);
    return_value = timer_start(timer_group, timer_idx);
    return return_value;
}

esp_err_t timer_get_counter_min(timer_group_t timer_group,timer_idx_t timer_idx,double *time_min){
    esp_err_t return_value;
    return_value = timer_get_counter_time_sec(timer_group,timer_idx,time_min);
    *time_min = *time_min/60;

    return return_value;
}

esp_err_t timer_get_counter_hour(timer_group_t timer_group,timer_idx_t timer_idx,double *time_hour){
    esp_err_t return_value;
    return_value = timer_get_counter_time_sec(timer_group,timer_idx,time_hour);
    *time_hour = *time_hour/3600;

    return return_value;
}
