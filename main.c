#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DVA_TEST.h"       // Include your test header
#include "DVA271_GPIO.h"    // Include GPIO functions
#include "DVA271_CPU.h"     // Include CPU functions
#include "DVA271_EEPROM.h"   // Include EEPROM functions

int main() {
    int choice;
    int flag = 0; // Set to 1 for verbose output
    char input[100]; // Buffer for user input

    // Initialize components
    if (init(flag) != 0) {
        fprintf(stderr, "Initialization failed. Exiting...\n");
        return EXIT_FAILURE;
    }

    do {
        // Display menu options
        printf("\nMenu:\n");
        printf("1. Test LEDs\n");
        printf("2. Test HC595 (Fake Temperature)\n");
        printf("3. Test Write to EEPROM\n");
        printf("4. Test Read from EEPROM\n");
        printf("5. Indicate CPU Temperature using HC595\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        // Get user input using fgets
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input. Please try again.\n");
            continue; // Restart the loop
        }

        // Parse the input
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            continue; // Restart the loop
        }

        switch (choice) {
            case 1:
                // Test LEDs
                printf("Testing LEDs...\n");
                test_leds();
                break;
            case 2: {
                // Test HC595 with a fake temperature
                int fake_temp;
                printf("Enter a fake temperature (0-255): ");
                if (fgets(input, sizeof(input), stdin) == NULL || sscanf(input, "%d", &fake_temp) != 1 || fake_temp < 0 || fake_temp > 255) {
                    printf("Invalid temperature. Please enter a number between 0 and 255.\n");
                    break; // Exit this case
                }
                test_hc595(fake_temp);
                break;
            }
            case 3:
                // Test writing to EEPROM
                printf("Testing write to EEPROM...\n");
                test_write();
                break;
            case 4:
                // Test reading from EEPROM
                printf("Testing read from EEPROM...\n");
                test_read();
                break;
            case 5:
                // Indicate CPU temperature using HC595
                printf("Indicating CPU temperature using HC595...\n");
                temp_indicate();
                break;
            case 6:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 6);

    return EXIT_SUCCESS;
}
