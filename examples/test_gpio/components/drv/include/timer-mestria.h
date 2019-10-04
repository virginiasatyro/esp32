#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"


 esp_err_t timer_config(uint32_t timer_divider,timer_count_dir_t counter_dir,bool en, bool alarm_en,bool auto_reload,timer_intr_mode_t intr_type,timer_group_t timer_goup,
        timer_idx_t timer_idx);

esp_err_t timer_start_count(timer_group_t timer_goup,timer_idx_t timer_idx,uint64_t load_value);


esp_err_t timer_get_counter_min(timer_group_t timer_goup,timer_idx_t timer_idx,double *time_min);


esp_err_t timer_get_counter_hour(timer_group_t timer_goup,timer_idx_t timer_idx,double *time_hour);
