#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For sleep function
#include "DVA_TEST.h"       // Include your test header
#include "DVA271_GPIO.h"    // Include GPIO functions
#include "DVA271_CPU.h"     // Include CPU functions
#include "DVA271_EEPROM.h"   // Include EEPROM functions

int main() {
    int flag = 0; // Set to 1 for verbose output

    // Initialize components
    if (init(flag) != 0) {
        fprintf(stderr, "Initialization failed. Exiting...\n");
        return EXIT_FAILURE;
    }

    // Test LEDs
    printf("Testing LEDs...\n");
    if (test_leds() != 0) {
        fprintf(stderr, "LED test failed.\n");
        return EXIT_FAILURE;
    }
    sleep(3); // Delay for 3 seconds

    // Test HC595 with a fake temperature (using a sample value)
    int fake_temp = 25; // Example fake temperature
    printf("Testing HC595 with fake temperature: %d...\n", fake_temp);
    if (test_hc595(fake_temp) != 0) {
        fprintf(stderr, "HC595 test failed.\n");
        return EXIT_FAILURE;
    }
    sleep(3); // Delay for 3 seconds

    // Test writing to EEPROM
    printf("Testing write to EEPROM...\n");
    if (test_write() != 0) {
        fprintf(stderr, "EEPROM write test failed.\n");
        return EXIT_FAILURE;
    }
    sleep(3); // Delay for 3 seconds

    // Test reading from EEPROM
    printf("Testing read from EEPROM...\n");
    if (test_read() != 0) {
        fprintf(stderr, "EEPROM read test failed.\n");
        return EXIT_FAILURE;
    }
    sleep(3); // Delay for 3 seconds

    // Indicate CPU temperature using HC595
    printf("Indicating CPU temperature using HC595...\n");
    if (temp_indicate() != 0) {
        fprintf(stderr, "CPU temperature indication failed.\n");
        return EXIT_FAILURE;
    }
    sleep(3); // Delay for 3 seconds

    printf("All tests completed successfully.\n");
    return EXIT_SUCCESS;
}
