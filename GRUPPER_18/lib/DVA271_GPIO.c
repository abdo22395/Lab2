#include "DVA271_GPIO.h"
#include <gpiod.h>
#include <stdio.h>

#define CHIP_NAME "gpiochip0"
#define LED_PIN 23

int hc595_init() {
    printf("HC595 initialized.\n");
    return 0;
}

int temp_indicate(int temperature) {
    if (temperature >= 85) {
        printf("All LEDs ON (Temperature: %d°C)\n", temperature);
    } else {
        printf("All LEDs OFF (Temperature: %d°C)\n", temperature);
    }
    return 0;
}

int gpio_init(bool output) {
    printf("GPIO initialized as %s.\n", output ? "OUTPUT" : "INPUT");
    return 0;
}

int flip_pin(int pin) {
    static int status = 0;
    status = !status;
    printf("Flipping pin %d to %d\n", pin, status);
    return 0;
}
