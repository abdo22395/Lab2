#include "DVA_TEST.h"
#include "DVA271_EEPROM.h"
#include "DVA271_GPIO.h"
#include <stdio.h>
#include <unistd.h>

int test_leds() {
    // Blink LEDs for 10 seconds (20 cycles of 250ms on/off)
    for (int i = 0; i < 20; i++) {
        set_led_state(1, 1);
        usleep(250000); // 250 ms on
        set_led_state(0, 0);
        usleep(250000); // 250 ms off
    }
    return 0;
}
