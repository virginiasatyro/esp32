#ifndef _TIMER_00
#define _TIMER_00

#include <stdio.h>
#include <stdlib.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void delay(__uint64_t delay_ms);

void delay_ms(__uint64_t delay_ms);

void delay_s(__uint64_t delay_s);

#endif // _TIMER_00