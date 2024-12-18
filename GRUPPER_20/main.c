#include "lib/DVA_TEST.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

// Function to handle interrupt signals
void cleanup(int signum) {
    printf("Exiting the program...\n");
    close_gpio_chip();
    exit(0);
}

int main(int argc, char *argv[]) {
    // Handle signals for proper cleanup
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    int flag = 1; // Enable verbose output

    int gpio_init_chip()

    // Initialize all systems
    if (init(flag) != 0) {
        printf("Initialization failed.\n");
        return 1;
    }

    // Test LEDs
    printf("Testing LEDs...\n");
    if (test_leds() != 0) {
        printf("LED test failed.\n");
    } else {
        printf("LED test succeeded.\n");
    }

    // Test HC595 with a fake temperature value
    printf("Testing HC595 with fake temperature...\n");
    if (test_hc595(50) != 0) {
        printf("HC595 test failed.\n");
    } else {
        printf("HC595 test succeeded.\n");
    }

    // Test writing to EEPROM
    printf("Testing EEPROM write...\n");
    if (test_write() != 0) {
        printf("EEPROM write test failed.\n");
    } else {
        printf("EEPROM write test succeeded.\n");
    }

    // Test reading from EEPROM
    printf("Testing EEPROM read...\n");
    if (test_read() != 0) {
        printf("EEPROM read test failed.\n");
    } else {
        printf("EEPROM read test succeeded.\n");
    }

    // Test multithreaded functionality
    printf("Testing multithread functionality...\n");
    if (multithread_fun() != 0) {
        printf("Multithread functionality test failed.\n");
    } else {
        printf("Multithread functionality test succeeded.\n");
    }

    // Clean up and close GPIO chip
    close_gpio_chip();

    return 0;
}
