#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

#include <stdio.h>
#include <string.h>

// Förbered för att läsa eller skriva från EEPROM
int eeprom_setup();

// Hämta skämt nummer <number>
int get_joke(int number, char **ptr);

// Skriv skämt till första lediga plats
int write_joke(char arr[255], int joke_length);

// Skriv skämt till specifik plats <pos>
int write_joke_pos(char arr[255], int joke_length, int pos);

// Rensa EEPROM upp till byte nummer ki_length * 1024
int clear_eeprom(int ki_length);

// Fyll EEPROM upp till byte nummer ki_length * 1024 med 1:or
int fill_eeprom(int ki_length);

#endif
