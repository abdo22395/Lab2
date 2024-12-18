// lib/DVA271_GPIO.h

#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <stdbool.h>

int hc595_init();
int temp_indicate(float temperature);
int gpio_init(int pin, bool output);
int flip_pin(int pin);
float get_cpu_temperature();
int set_led_state(int led1, int led2);

#endif // DVA271_GPIO_H
