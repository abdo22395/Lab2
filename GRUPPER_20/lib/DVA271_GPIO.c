// lib/DVA271_GPIO.c

#include "DVA271_GPIO.h"
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Definiera GPIO-chip
#define CHIP_NAME "gpiochip0"

// GPIO-pinnar för HC595
#define HC595_DATA_PIN 17
#define HC595_LATCH_PIN 27
#define HC595_CLOCK_PIN 22

static struct gpiod_chip *chip;
static struct gpiod_line *data_line;
static struct gpiod_line *latch_line;
static struct gpiod_line *clock_line;

// Initialisera GPIO-chip
int gpio_init_chip() {
    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }
    return 0;
}

// Initialisera en GPIO pin
int gpio_init(int pin, bool output) {
    struct gpiod_line *line = gpiod_chip_get_line(chip, pin);
    if (!line) {
        perror("Failed to get GPIO line");
        return 1;
    }
    int ret;
    if (output) {
        ret = gpiod_line_request_output(line, "gpio_app", 0);
    } else {
        ret = gpiod_line_request_input(line, "gpio_app");
    }
    if (ret < 0) {
        perror("Failed to request GPIO line");
        return 2;
    }
    return 0;
}

// Initialisera HC595 shift register
int hc595_init() {
    // Initialisera pinnar som output
    if (gpio_init(HC595_DATA_PIN, true) != 0) return 1;
    if (gpio_init(HC595_LATCH_PIN, true) != 0) return 1;
    if (gpio_init(HC595_CLOCK_PIN, true) != 0) return 1;

    // Hämta linjer
    data_line = gpiod_chip_get_line(chip, HC595_DATA_PIN);
    latch_line = gpiod_chip_get_line(chip, HC595_LATCH_PIN);
    clock_line = gpiod_chip_get_line(chip, HC595_CLOCK_PIN);

    if (!data_line || !latch_line || !clock_line) {
        perror("Failed to get HC595 lines");
        return 2;
    }

    // Initiera shift register till noll
    gpiod_line_set_value(latch_line, 0);
    gpiod_line_set_value(clock_line, 0);
    gpiod_line_set_value(data_line, 0);

    return 0;
}

// Skicka ett byte till HC595
int hc595_send_byte(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        int bit = (byte >> i) & 1;
        if (gpiod_line_set_value(data_line, bit) < 0) {
            perror("Failed to set data line");
            return 1;
        }

        // Skicka en klockpulssignal
        if (gpiod_line_set_value(clock_line, 1) < 0) {
            perror("Failed to set clock high");
            return 2;
        }
        usleep(10); // Kort delay
        if (gpiod_line_set_value(clock_line, 0) < 0) {
            perror("Failed to set clock low");
            return 3;
        }
    }

    // Latch the data
    if (gpiod_line_set_value(latch_line, 1) < 0) {
        perror("Failed to set latch high");
        return 4;
    }
    usleep(10); // Kort delay
    if (gpiod_line_set_value(latch_line, 0) < 0) {
        perror("Failed to set latch low");
        return 5;
    }

    return 0;
}

// Indikera temperatur genom att sätta register
int temp_indicate(float temperature) {
    unsigned char register_value = 0;

    if (temperature >= 85.0) {
        register_value = 0xFF; // Alla LEDs på
    } else {
        register_value = 0x00; // Alla LEDs av
    }

    return hc595_send_byte(register_value);
}

// Flippa en GPIO pin
int flip_pin(int pin) {
    struct gpiod_line *line = gpiod_chip_get_line(chip, pin);
    if (!line) {
        perror("Failed to get GPIO line");
        return 1;
    }

    int value = gpiod_line_get_value(line);
    if (value < 0) {
        perror("Failed to get GPIO line value");
        return 2;
    }

    if (gpiod_line_set_value(line, !value) < 0) {
        perror("Failed to flip GPIO line value");
        return 3;
    }

    return 0;
}

// Stäng GPIO-chip
void close_gpio_chip() {
    if (chip) {
        gpiod_chip_close(chip);
    }
}
