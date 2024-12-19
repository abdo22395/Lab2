#include "DVA271_GPIO.h"
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Define GPIO pin numbers
#define DATA_PIN 17
#define LATCH_PIN 27
#define CLOCK_PIN 22
#define LED1_PIN 23
#define LED2_PIN 24

#define CHIP_NAME "gpiochip0"

static struct gpiod_chip *chip;
static struct gpiod_line *data_line, *latch_line, *clock_line;
static struct gpiod_line *led1_line, *led2_line;

int hc595_init() {
    // Open the GPIO chip
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    // Get lines (pins) from the GPIO chip
    data_line = gpiod_chip_get_line(chip, DATA_PIN);
    latch_line = gpiod_chip_get_line(chip, LATCH_PIN);
    clock_line = gpiod_chip_get_line(chip, CLOCK_PIN);
    led1_line = gpiod_chip_get_line(chip, LED1_PIN);
    led2_line = gpiod_chip_get_line(chip, LED2_PIN);

    // Check if lines were retrieved successfully
    if (!data_line || !latch_line || !clock_line || !led1_line || !led2_line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 1;
    }

    // Request all lines as output and set initial values to 0 (off)
    if (gpiod_line_request_output(data_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(latch_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(clock_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(led1_line, "LED1", 0) < 0 ||
        gpiod_line_request_output(led2_line, "LED2", 0) < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(chip);
        return 1;
    }

    return 0;
}

int flip_pin(int pin) {
    struct gpiod_line *line;
    // Determine which LED line we want to flip
    if (pin == LED1_PIN) {
        line = led1_line;
    } else if (pin == LED2_PIN) {
        line = led2_line;
    } else {
        return 1;
    }

    // Read the current value of the pin and invert it
    int value = gpiod_line_get_value(line);
    if (value < 0) return 1;
    gpiod_line_set_value(line, !value);
    return 0;
}

int set_led_state(int led1, int led2) {
    // Set the LEDs to the specified states (0 or 1)
    if (gpiod_line_set_value(led1_line, led1) < 0) return 1;
    if (gpiod_line_set_value(led2_line, led2) < 0) return 1;
    return 0;
}
