// lib/DVA_TEST.c

#include "DVA_TEST.h"
#include "DVA271_EEPROM.h"
#include "DVA271_GPIO.h"
#include <stdio.h>
#include <unistd.h>

// Variabel för verbose-läge
static int verbose_mode = 0;

int init(int flag) {
    verbose_mode = flag;

    // Initialisera EEPROM
    if (eeprom_setup() != 0) {
        if (verbose_mode) printf("eeprom_setup returned FAILURE\n");
        return 1;
    }
    if (verbose_mode) printf("eeprom_setup returned SUCCESS\n");

    // Initialisera HC595
    if (hc595_init() != 0) {
        if (verbose_mode) printf("hc595_init returned FAILURE\n");
        return 2;
    }
    if (verbose_mode) printf("hc595_init returned SUCCESS\n");

    return 0;
}

int test_leds() {
    // Blinkar LEDs under 10 sekunder
    for (int i = 0; i < 20; i++) {
        set_led_state(1, 1);
        usleep(250000); // 250 ms
        set_led_state(0, 0);
        usleep(250000); // 250 ms
    }
    return 0;
}

int test_hc595(int fake_temp) {
    // Sätter registret med fake_temp
    if (temp_indicate((float)fake_temp) != 0) {
        return 1;
    }
    return 0;
}

int test_write() {
    // Skriver ett förbestämt skämt till EEPROM
    char joke[255] = "Detta är ett testskämt.";
    if (write_joke(joke, strlen(joke)) != 0) {
        return 1;
    }
    return 0;
}

int test_read() {
    // Läser det första skämtet från EEPROM
    char* joke;
    if (get_joke(0, &joke) != 0) {
        return 1;
    }
    printf("Testläsning av skämt: %s\n", joke);
    free(joke);
    return 0;
}

int multithread_fun() {
    // Denna funktion är implementerad i main.c
    // Här kan du kalla andra funktioner för testning om det behövs
    return 0;
}
