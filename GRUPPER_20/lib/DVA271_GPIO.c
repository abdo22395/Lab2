// lib/DVA271_GPIO.c

#include "DVA271_GPIO.h"
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Definiera GPIO-pinnar
#define DATA_PIN 17
#define LATCH_PIN 27
#define CLOCK_PIN 22
#define LED1_PIN 23
#define LED2_PIN 24
#define WP_PIN 25


#define CHIP_NAME "gpiochip0"

static struct gpiod_chip *chip;
static struct gpiod_line *data_line, *latch_line, *clock_line;
static struct gpiod_line *led1_line, *led2_line;
static struct gpiod_line *wp_line;

int wp_init() {
    wp_line = gpiod_chip_get_line(chip, WP_PIN);
    if (!wp_line) {
        perror("gpiod_chip_get_line");
        return 1;
    }
    if (gpiod_line_request_output(wp_line, "EEPROM_WP", 0) < 0) {
        perror("gpiod_line_request_output");
        return 1;
    }
    return 0;
}

int set_wp(bool enable) {
    if (!wp_line) return 1;
    return gpiod_line_set_value(wp_line, enable ? 1 : 0);
}

int hc595_init() {
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    data_line = gpiod_chip_get_line(chip, DATA_PIN);
    latch_line = gpiod_chip_get_line(chip, LATCH_PIN);
    clock_line = gpiod_chip_get_line(chip, CLOCK_PIN);
    led1_line = gpiod_chip_get_line(chip, LED1_PIN);
    led2_line = gpiod_chip_get_line(chip, LED2_PIN);

    if (!data_line || !latch_line || !clock_line || !led1_line || !led2_line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 1;
    }

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

int temp_indicate(float temperature) {
    // Här kan du implementera hur temperaturen representeras via HC595
    // Exempel: Skicka binärvärdet av temperaturen till registret
    unsigned int temp_int = (unsigned int)temperature;
    for (int i = 0; i < 8; i++) {
        int bit = (temp_int >> i) & 1;
        gpiod_line_set_value(data_line, bit);
        gpiod_line_set_value(clock_line, 1);
        usleep(1000);
        gpiod_line_set_value(clock_line, 0);
    }

    // Latch the data
    gpiod_line_set_value(latch_line, 1);
    usleep(1000);
    gpiod_line_set_value(latch_line, 0);

    return 0;
}

int gpio_init(int pin, bool output) {
    // Denna funktion används inte i detta exempel, men kan implementeras vid behov
    return 0;
}

int flip_pin(int pin) {
    struct gpiod_line *line;
    if (pin == LED1_PIN) {
        line = led1_line;
    } else if (pin == LED2_PIN) {
        line = led2_line;
    } else {
        return 1;
    }

    int value = gpiod_line_get_value(line);
    if (value < 0) return 1;
    gpiod_line_set_value(line, !value);
    return 0;
}

float get_cpu_temperature() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) {
        perror("fopen");
        return -1.0;
    }
    int temp_milli;
    fscanf(fp, "%d", &temp_milli);
    fclose(fp);
    return temp_milli / 1000.0;
}

int set_led_state(int led1, int led2) {
    if (gpiod_line_set_value(led1_line, led1) < 0) return 1;
    if (gpiod_line_set_value(led2_line, led2) < 0) return 1;
    return 0;
}
