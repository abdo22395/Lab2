#include "DVA271_EEPROM.h"
#include <wiringPiI2C.h>
#include <stdio.h>
#include <unistd.h>

static int eeprom_fd = -1; // File descriptor for the EEPROM

// Initialize the EEPROM for reading or writing
int eeprom_setup() {
    if (eeprom_fd == -1) {
        eeprom_fd = wiringPiI2CSetup(EEPROM_ADDRESS);
        if (eeprom_fd == -1) {
            fprintf(stderr, "Failed to initialize I2C communication with EEPROM at address 0x%02X\n", EEPROM_ADDRESS);
            return -1; // Return -1 on failure
        }
    }
    return 0; // Return 0 on success
}