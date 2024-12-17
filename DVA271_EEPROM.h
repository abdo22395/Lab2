#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

#include <wiringPi.h>
#include <stdint.h>

// Constants for EEPROM
#define EEPROM_ADDRESS 0x50  // Change this to your EEPROM's I2C address

// Function prototypes
int eeprom_setup(); // Prepare to write or read from EEPROM

#endif // DVA271_EEPROM_H