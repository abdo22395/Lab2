#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <stdbool.h>

int hc595_init();
int flip_pin(int pin);
int set_led_state(int led1, int led2);

#endif // DVA271_GPIO_H
