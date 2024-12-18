#include "DVA271_EEPROM.h"
#include "wiringPiI2C.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>


static int eeprom_fd = -1; // File descriptor for the EEPROM

const char *jokes[] = {
    "Why don't scientists trust atoms? Because they make up everything!",
    "Why did the scarecrow win an award? Because he was outstanding in his field!",
    "Why don't skeletons fight each other? They don't have the guts!",
    "What do you call fake spaghetti? An impasta!",
    "Why did the bicycle fall over? Because it was two-tired!",
    "What do you call cheese that isn't yours? Nacho cheese!",
    "Why did the math book look sad? Because it had too many problems!",
    "What do you call a bear with no teeth? A gummy bear!",
    "Why can't you give Elsa a balloon? Because she will let it go!",
    "What do you call a fish wearing a bowtie? Sofishticated!"
};


int eeprom_write(int address, uint8_t *data, int length) {
    if (length <= 0 || length > 32) { // Assuming a maximum write length of 32 bytes
        return -1; // Return -1 for invalid length
    }

    // Write data to EEPROM
    for (int i = 0; i < length; i++) {
        wiringPiI2CWriteReg8(eeprom_fd, address + i, data[i]);
    }

    return 0; // Return 0 on success
}

// Function to read data from EEPROM
int eeprom_read(int address, uint8_t *buffer, int length) {
    if (length <= 0 || length > 32) { // Assuming a maximum read length of 32 bytes
        return -1; // Return -1 for invalid length
    }

    // Read data from EEPROM
    for (int i = 0; i < length; i++) {
        buffer[i] = wiringPiI2CReadReg8(eeprom_fd, address + i);
    }

    return 0; // Return 0 on success
}
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

int get_joke(int number, char **ptr) {
    if (number < 0 || number >= sizeof(jokes) / sizeof(jokes[0])) {
        return -1; // Return -1 for invalid joke number
    }
    *ptr = (char *)jokes[number]; // Set the pointer to the joke
    return 0; // Return 0 on success
}

int write_joke(char arr[JOKE_LENGTH], int joke_length) {
    if (joke_length > JOKE_LENGTH) {
        return -1; // Return -1 if the joke is too long
    }

    // Find the first empty space in the EEPROM
    for (int i = 0; i < MAX_JOKES; i++) {
        // Read the current joke from EEPROM
        char current_joke[JOKE_LENGTH];
        for (int j = 0; j < joke_length; j++) {
            current_joke[j] = wiringPiI2CReadReg8(eeprom_fd, i * JOKE_LENGTH + j);
        }
        current_joke[joke_length] = '\0'; // Null-terminate the string

        // Check if the current joke is empty (assuming empty jokes are represented by null characters)
        if (strlen(current_joke) == 0) {
            // Write the new joke to this position
            for (int j = 0; j < joke_length; j++) {
                wiringPiI2CWriteReg8(eeprom_fd, i * JOKE_LENGTH + j, arr[j]);
            }
            return 0; // Return 0 on success
        }
    }

    return -2; // Return -2 if EEPROM is full
}

int write_joke_pos(char arr[JOKE_LENGTH], int joke_length, int pos) {
    if (joke_length > JOKE_LENGTH) {
        return -1; // Return -1 if the joke is too long
    }
    if (pos < 0 || pos >= MAX_JOKES) {
        return -2; // Return -2 if the position is out of bounds
    }

    // Calculate the starting address in EEPROM for the joke
    int start_address = pos * JOKE_LENGTH;

    // Write the joke to the specified position
    for (int j = 0; j < joke_length; j++) {
        wiringPiI2CWriteReg8(eeprom_fd, start_address + j, arr[j]);
    }

    // Optionally, write null terminator to ensure the joke is properly terminated
    wiringPiI2CWriteReg8(eeprom_fd, start_address + joke_length, '\0');

    return 0; // Return 0 on success
}

int clear_eeprom(int ki_length) {
    // Calculate the total number of bytes to clear
    int total_bytes = ki_length * 1024;

    // Write zeros to each byte in the specified range
    for (int i = 0; i < total_bytes; i++) {
        wiringPiI2CWriteReg8(eeprom_fd, i, 0x00); // Write 0 to each byte
    }

    return 0; // Return 0 on success
}

int fill_eeprom(int ki_length) {
    // Calculate the total number of bytes to fill
    int total_bytes = ki_length * 1024;

    // Write 1 to each byte in the specified range
    for (int i = 0; i < total_bytes; i++) {
        wiringPiI2CWriteReg8(eeprom_fd, i, 0x01); // Write 1 to each byte
    }

    return 0; // Return 0 on success
}
