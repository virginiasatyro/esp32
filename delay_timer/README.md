# Delay Timer 

In this project I made the first library called: ```timer-00.h```. 

It is a simple way to learn how to make esp32 libraries. 

The functions created are:

```void delay(__uint64_t delay_ms);```

```void delay_ms(__uint64_t delay_ms);```

```void delay_s(__uint64_t delay_s);```

Importante function:

```void vTaskDelay( const TickType_t xTicksToDelay );```

> Delay a task for a given number of ticks. The actual time that the task remains blocked depends on the tick rate. The constant portTICK_PERIOD_MS can be used to calculate real time from the tick rate – with the resolution of one tick period.

> vTaskDelay() specifies a time at which the task wishes to unblock relative to the time at which vTaskDelay() is called. For example, specifying a block period of 100 ticks will cause the task to unblock 100 ticks after vTaskDelay() is called. vTaskDelay() does not therefore provide a good method of controlling the frequency of a periodic task as the path taken through the code, as well as other task and interrupt activity, will effect the frequency at which vTaskDelay() gets called and therefore the time at which the task next executes.

[freeRTOS](https://www.freertos.org/a00127.html)