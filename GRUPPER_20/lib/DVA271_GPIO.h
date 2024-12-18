#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <stdbool.h>
#include <gpiod.h>

// Funktioner för GPIO-hantering
int gpio_init_chip();
int hc595_init();
int temp_indicate(float temperature);
int gpio_init();
int flip_pin(int pin);
int hc595_send_byte(unsigned char byte);
int set_pin_value(int pin, int value); // Ny funktion för att sätta ett specifikt värde
void close_gpio_chip();

// Accessor-funktioner för shift register linjer
struct gpiod_line* get_data_line();
struct gpiod_line* get_latch_line();
struct gpiod_line* get_clock_line();

// Accessor-funktioner för LED linjer
struct gpiod_line* get_led23_line();
struct gpiod_line* get_led24_line();

#endif // DVA271_GPIO_H
