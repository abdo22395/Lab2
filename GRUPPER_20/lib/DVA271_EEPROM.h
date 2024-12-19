#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Adjust these as per your EEPROM specifications
#define EEPROM_ADDRESS 0x50
#define EEPROM_SIZE 32768
#define PAGE_SIZE 64

// Initialize the EEPROM (open I2C bus, set WP pin)
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

// External functions assumed to be implemented elsewhere:
int wp_init();
void set_wp(bool enable);

#ifdef __cplusplus
}
#endif

#endif // DVA271_EEPROM_H
