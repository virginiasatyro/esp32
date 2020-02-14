# Blink
 
In the program, we introduce de gpio library. It is used to set the led built in as an output and turn it on and off.

```gpio_configuration(GPIO_PIN_INTR_DISABLE, GPIO_MODE_OUTPUT, GPIO_OUTPUT_PIN_SEL);```

```gpio_digital_write(LED_BUILT_IN, ON);```

```gpio_digital_write(LED_BUILT_IN, OFF);```

```delay(1000);```

[Exppressif](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/gpio.html#_CPPv414gpio_get_level10gpio_num_t)