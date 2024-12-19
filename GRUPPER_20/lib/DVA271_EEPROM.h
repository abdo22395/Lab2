#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

#include <stdbool.h>

// For a 24C08-like EEPROM (8 Kbit):
// Total size = 1024 bytes
// Organized as 4 blocks of 256 bytes each at addresses 0x50,0x51,0x52,0x53
#define EEPROM_SIZE 1024
#define BLOCK_SIZE 256
#define PAGE_SIZE 16   // Typical page size for small EEPROM

int eeprom_setup();

// Write a joke (string) to EEPROM at position 0
int write_joke(char arr[255], int joke_length);

// Write a joke at a specified position (each joke 255 bytes)
int write_joke_pos(char arr[255], int joke_length, int pos);

// Read a joke from a given position
int get_joke(int number, char **ptr);

// Clear a section of the EEPROM (fill with zeros)
int clear_eeprom(int ki_length);

// Fill a section of the EEPROM with byte value 1 (for testing)
int fill_eeprom(int ki_length);

#endif // DVA271_EEPROM_H
