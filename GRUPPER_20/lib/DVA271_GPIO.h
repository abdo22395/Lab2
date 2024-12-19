#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <stdbool.h>

int hc595_init();
int temp_indicate(float temperature);
int gpio_init(int pin, bool output);
int flip_pin(int pin);
float get_cpu_temperature();
int set_led_state(int led1, int led2);

// New functions for Write Protect (WP) handling
int wp_init();       // Initialize the WP pin
int set_wp(bool enable); // Enable or disable WP (true = enable, false = disable)

#endif // DVA271_GPIO_H
