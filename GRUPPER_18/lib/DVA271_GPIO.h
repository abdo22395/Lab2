#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <stdbool.h>
#include <stdio.h>

// Initiera HC595 Shift Register
int hc595_init();

// Sätt register för att indikera CPU-temperatur
int temp_indicate();

// Initiera GPIO pin som output/input
int gpio_init(bool output);

// Flippa digital GPIO pin
int flip_pin(int pin);

#endif
