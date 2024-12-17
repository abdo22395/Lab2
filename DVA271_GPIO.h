#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <wiringPi.h>
#include "DVA271_CPU.h"
#include <stdbool.h>

// Function prototypes
int hc595_init(); // Initialize the HC595 shift register

int temp_indicate();

int gpio_init(int pin, bool output); // Initialize GPIO pin

int flip_pin(int pin);     // Flip the state of a digital pin

#endif // DVA271_GPIO_H