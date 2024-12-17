#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

#include <wiringPi.h>
#include <stdint.h>

// Constants for EEPROM
#define EEPROM_ADDRESS 0x50  // Change this to your EEPROM's I2C address
#define MAX_JOKES 10          // Maximum number of jokes
#define JOKE_LENGTH 255       // Maximum length of each joke

// Function prototypes
int eeprom_setup(); // Prepare to write or read from EEPROM

int get_joke(int number, char **ptr);

int write_joke(char arr[JOKE_LENGTH], int joke_length); // Write a joke to EEPROM

int write_joke_pos(char arr[JOKE_LENGTH], int joke_length, int pos); // Write a joke to a specific position in EEPROM

int clear_eeprom(int ki_length); // Clear EEPROM up to ki_length kibibytes

int fill_eeprom(int ki_length);  // Fill EEPROM with 1 up to ki_length kibibytes




#endif // DVA271_EEPROM_H