// lib/DVA_TEST.c

#include "DVA_TEST.h"
#include "DVA271_EEPROM.h"
#include "DVA271_GPIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Globala variabler och mutex
pthread_mutex_t eeprom_mutex;

// Funktion för att initiera och sätta mutex
int init(int flag) {
    int ret = eeprom_setup();
    if (flag) {
        if (ret == 0) printf("eeprom_setup returned SUCCESS\n");
        else printf("eeprom_setup returned %d\n", ret);
    }
    if (ret != 0) return ret;

    ret = gpio_init_chip();
    if (flag) {
        if (ret == 0) printf("gpio_init_chip returned SUCCESS\n");
        else printf("gpio_init_chip returned %d\n", ret);
    }
    if (ret != 0) return ret;

    ret = hc595_init();
    if (flag) {
        if (ret == 0) printf("hc595_init returned SUCCESS\n");
        else printf("hc595_init returned %d\n", ret);
    }
    if (ret != 0) return ret;

    // Initiera GPIO LEDs
    if (gpio_init(23, true) != 0) {
        if (flag) printf("gpio_init for pin 23 failed\n");
        return 1;
    }
    if (gpio_init(24, true) != 0) {
        if (flag) printf("gpio_init for pin 24 failed\n");
        return 1;
    }

    // Initiera mutex
    pthread_mutex_init(&eeprom_mutex, NULL);
    set_eeprom_mutex(&eeprom_mutex);

    return 0;
}

// Testa LEDs genom att blinka dem i 10 sekunder
int test_leds() {
    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        // Flip LED på GPIO 23
        flip_pin(23);
        // Flip LED på GPIO 24
        flip_pin(24);
        usleep(500000); // 500ms

        // Kontrollera tiden
        clock_gettime(CLOCK_MONOTONIC, &current);
        double elapsed = (current.tv_sec - start.tv_sec) +
                         (current.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= 10.0) break;
    }

    // Stäng av LEDs genom att sätta dem till låg värde
    if (gpiod_line_set_value(gpiod_chip_get_line(chip, 23), 0) < 0) {
        perror("Failed to set GPIO 23 to low");
    }
    if (gpiod_line_set_value(gpiod_chip_get_line(chip, 24), 0) < 0) {
        perror("Failed to set GPIO 24 to low");
    }

    return 0;
}

// Resten av DVA_TEST.c förblir oförändrad
