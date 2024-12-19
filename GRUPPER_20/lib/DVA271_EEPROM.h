#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Adjust these as per your EEPROM specifications
#define EEPROM_SIZE 32768     // Total size (32KB example)
#define PAGE_SIZE 64          // Page size of the EEPROM

// Initialize the EEPROM (open I2C bus)
int eeprom_setup();

// Write a joke (string) to the EEPROM at position 0
int write_joke(char arr[255], int joke_length);

// Write a joke at a specified position
int write_joke_pos(char arr[255], int joke_length, int pos);

// Read a joke from a given position
int get_joke(int number, char **ptr);

// Clear a section of the EEPROM (fill with zeroes)
int clear_eeprom(int ki_length);

// Fill a section of the EEPROM with byte value 1 (for testing)
int fill_eeprom(int ki_length);

#ifdef __cplusplus
}
#endif

#endif // DVA271_EEPROM_H
