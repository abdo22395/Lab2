// lib/DVA_TEST.c

#include "DVA_TEST.h"
#include "DVA271_EEPROM.h"
#include "DVA271_GPIO.h"
#include <stdio.h>
#include <unistd.h>

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
