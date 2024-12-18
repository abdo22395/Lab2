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

// GPIO-pinnar för LEDs
#define LED_PIN_23 23
#define LED_PIN_24 24

static struct gpiod_chip *chip;
static struct gpiod_line *data_line;
static struct gpiod_line *latch_line;
static struct gpiod_line *clock_line;
static struct gpiod_line *led23_line;
static struct gpiod_line *led24_line;

// Initialisera GPIO-chip
int gpio_init_chip() {
    chip = gpiod_chip_open_by_name(CHIP_NAME);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }
    return 0;
}

// Initialisera en GPIO pin som input eller output
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

    // Spara linjepekaren om det är en känd pin
    switch(pin) {
        case HC595_DATA_PIN:
            data_line = line;
            break;
        case HC595_LATCH_PIN:
            latch_line = line;
            break;
        case HC595_CLOCK_PIN:
            clock_line = line;
            break;
        case LED_PIN_23:
            led23_line = line;
            break;
        case LED_PIN_24:
            led24_line = line;
            break;
        default:
            // Okänd pin, gör inget
            break;
    }

    return 0;
}

// Initialisera HC595 shift register
int hc595_init() {
    // Kontrollera att HC595 linjer är initialiserade
    if (!data_line || !latch_line || !clock_line) {
        fprintf(stderr, "HC595 lines not initialized\n");
        return 1;
    }

    // Initiera shift register till noll
    if (gpiod_line_set_value(latch_line, 0) < 0) {
        perror("Failed to set latch line low");
        return 2;
    }
    if (gpiod_line_set_value(clock_line, 0) < 0) {
        perror("Failed to set clock line low");
        return 2;
    }
    if (gpiod_line_set_value(data_line, 0) < 0) {
        perror("Failed to set data line low");
        return 2;
    }

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

// Indikera temperatur via HC595
int temp_indicate(float temperature) {
    unsigned char register_value = 0;

    if (temperature >= 85.0) {
        register_value = 0xFF; // Alla LEDs på
    } else {
        register_value = 0x00; // Alla LEDs av
    }

    return hc595_send_byte(register_value);
}

// Flip a GPIO pin
int flip_pin(int pin) {
    struct gpiod_line *line = NULL;

    switch(pin) {
        case HC595_DATA_PIN:
            line = data_line;
            break;
        case HC595_LATCH_PIN:
            line = latch_line;
            break;
        case HC595_CLOCK_PIN:
            line = clock_line;
            break;
        case LED_PIN_23:
            line = led23_line;
            break;
        case LED_PIN_24:
            line = led24_line;
            break;
        default:
            fprintf(stderr, "Unknown pin: %d\n", pin);
            return 1;
    }

    if (!line) {
        fprintf(stderr, "GPIO line for pin %d not initialized\n", pin);
        return 2;
    }

    int value = gpiod_line_get_value(line);
    if (value < 0) {
        perror("Failed to get GPIO line value");
        return 3;
    }

    if (gpiod_line_set_value(line, !value) < 0) {
        perror("Failed to flip GPIO line value");
        return 4;
    }

    return 0;
}

// Sätt ett GPIO pin till ett specifikt värde
int set_pin_value(int pin, int value) {
    struct gpiod_line *line = NULL;

    switch(pin) {
        case HC595_DATA_PIN:
            line = data_line;
            break;
        case HC595_LATCH_PIN:
            line = latch_line;
            break;
        case HC595_CLOCK_PIN:
            line = clock_line;
            break;
        case LED_PIN_23:
            line = led23_line;
            break;
        case LED_PIN_24:
            line = led24_line;
            break;
        default:
            fprintf(stderr, "Unknown pin: %d\n", pin);
            return 1;
    }

    if (!line) {
        fprintf(stderr, "GPIO line for pin %d not initialized\n", pin);
        return 2;
    }

    if (gpiod_line_set_value(line, value) < 0) {
        perror("Failed to set GPIO line value");
        return 3;
    }

    return 0;
}

// Accessor-funktioner för shift register linjer
struct gpiod_line* get_data_line() {
    return data_line;
}

struct gpiod_line* get_latch_line() {
    return latch_line;
}

struct gpiod_line* get_clock_line() {
    return clock_line;
}

// Accessor-funktioner för LED linjer
struct gpiod_line* get_led23_line() {
    return led23_line;
}

struct gpiod_line* get_led24_line() {
    return led24_line;
}

// Stäng GPIO-chip
void close_gpio_chip() {
    if (chip) {
        gpiod_chip_close(chip);
        chip = NULL;
    }
}
