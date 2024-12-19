#ifndef DVA271_EEPROM_H
#define DVA271_EEPROM_H

int eeprom_setup();
int get_joke(int number, char **ptr);
int write_joke(char arr[255], int joke_length);
int write_joke_pos(char arr[255], int joke_length, int pos);
int clear_eeprom(int ki_length);
int fill_eeprom(int ki_length);

// Optional: Add WP control functions here for direct EEPROM-level abstraction
// These would call the GPIO functions internally if needed
// int enable_eeprom_wp();
// int disable_eeprom_wp();

#endif // DVA271_EEPROM_H
