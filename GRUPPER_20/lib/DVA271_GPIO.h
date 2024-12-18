#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <stdbool.h>

int gpio_init_chip();
int hc595_init();
int temp_indicate(float temperature);
int gpio_init(int pin, bool output);
int flip_pin(int pin);
void close_gpio_chip();

#endif // DVA271_GPIO_H
