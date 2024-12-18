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
    test_leds();
    sleep(3); // Delay for 3 seconds

    // Test HC595 with a fake temperature (using a sample value)
    int fake_temp = 25; // Example fake temperature
    printf("Testing HC595 with fake temperature: %d...\n", fake_temp);
    test_hc595(fake_temp);
    sleep(3); // Delay for 3 seconds

    // Test writing to EEPROM
    printf("Testing write to EEPROM...\n");
    test_write();
    sleep(3); // Delay for 3 seconds

    // Test reading from EEPROM
    printf("Testing read from EEPROM...\n");
    test_read();
    sleep(3); // Delay for 3 seconds

    // Indicate CPU temperature using HC595
    printf("Indicating CPU temperature using HC595...\n");
    temp_indicate();
    sleep(3); // Delay for 3 seconds

    printf("All tests completed.\n");
    return 0;
}
