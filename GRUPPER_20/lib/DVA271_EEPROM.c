// lib/DVA271_EEPROM.c

#include "DVA271_EEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define EEPROM_ADDRESS 0x51 // Adjust according to your EEPROM address
#define EEPROM_SIZE 32768    // Example size, adjust according to your EEPROM
#define PAGE_SIZE 64         // Example page size, adjust according to your EEPROM

static int file;

// Funktion för att skriva till I2C
static int i2c_write_data(unsigned short address, unsigned char *data, int length) {
    unsigned char buffer[2 + length];
    buffer[0] = (address >> 8) & 0xFF; // Höga byte av adress
    buffer[1] = address & 0xFF;        // Låga byte av adress
    memcpy(&buffer[2], data, length);

    if (write(file, buffer, 2 + length) != (2 + length)) {
        perror("Failed to write to the i2c bus");
        return -1;
    }

    // EEPROM skrivtid - justera efter din EEPROM-spec
    usleep(500000); // 50 ms
    return 0;
}

// Funktion för att läsa från I2C med korrekt upprepat startvillkor (repeated start)
static int i2c_read_data(unsigned short address, unsigned char *data, int length) {
    unsigned char addr_buffer[2];
    addr_buffer[0] = (address >> 8) & 0xFF;
    addr_buffer[1] = address & 0xFF;

    // We will use I2C_RDWR ioctl with two messages:
    // Message 1: Write the address bytes
    // Message 2: Read the requested number of bytes
    // This ensures a repeated start condition required by the EEPROM.

    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    messages[0].addr  = EEPROM_ADDRESS;
    messages[0].flags = 0;             // Write
    messages[0].len   = 2;
    messages[0].buf   = addr_buffer;

    messages[1].addr  = EEPROM_ADDRESS;
    messages[1].flags = I2C_M_RD;      // Read
    messages[1].len   = length;
    messages[1].buf   = data;

    packets.msgs      = messages;
    packets.nmsgs     = 2;

    if (ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Failed to read from the i2c bus");
        return -1;
    }

    return 0;
}

int eeprom_setup() {
    char *bus = "/dev/i2c-1";
    if ((file = open(bus, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return 1;
    }
    if (ioctl(file, I2C_SLAVE, EEPROM_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return 2;
    }

    return 0;
}

int get_joke(int number, char **ptr) {
    if (number < 0) return -1;

    unsigned short address = number * 255;
    if (address + 255 > EEPROM_SIZE) {
        printf("Error: Address out of range\n");
        return -1;
    }

    unsigned char buffer[255]; // We'll read exactly 255 bytes of the joke
    // Read the data from EEPROM into buffer
    if (i2c_read_data(address, buffer, 255) != 0) {
        printf("Error: Failed to read the joke from EEPROM\n");
        return -1;
    }

    // Allocate memory to return the joke
    *ptr = malloc(255); // 255 bytes + null terminator
    if (*ptr == NULL) {
        printf("Error: Memory allocation failed\n");
        return -1;
    }

    // Copy the data into the allocated memory and add a null terminator
    memcpy(*ptr, buffer, 255);
    (*ptr)[255] = '\0';

    return 0; // Success
}

int write_joke(char arr[255], int joke_length) {
    if (joke_length > 255) {
        printf("Error: joke_length exceeds buffer size of 255.\n");
        return 1;
    }

    // Zero out the entire array before writing
    memset(arr + joke_length, 0, 255 - joke_length);

    int result = write_joke_pos(arr, 255, 0); // Always write full 255 bytes
    return result;
}


int write_joke_pos(char arr[255], int joke_length, int pos) {
    // Check if the start position is valid
    if (pos < 0) {
        printf("Error: Invalid position (negative)\n");
        return 1;
    }

    unsigned short address = pos * 255;
    // Check if the end of the data would exceed EEPROM bounds
    if (address + joke_length > EEPROM_SIZE) {
        printf("Error: Address out of EEPROM range\n");
        return 1;
    }

    int remaining = joke_length;
    int offset = 0;

    while (remaining > 0) {
        unsigned char buffer[PAGE_SIZE];

        // Determine the number of bytes to write in this chunk
        int write_length = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;

        // Copy data into the buffer
        memcpy(buffer, arr + offset, write_length);

        // Write the buffer to the EEPROM
        if (i2c_write_data(address + offset, buffer, write_length) != 0) {
            printf("Error: Failed to write to EEPROM at address 0x%04X\n", address + offset);
            return 1;
        }

        // Update for next iteration
        offset += write_length;
        remaining -= write_length;
    }

    return 0; // Success
}

int clear_eeprom(int ki_length) {
    unsigned char *buffer = malloc(ki_length);
    if (!buffer) return 1;
    memset(buffer, 0, ki_length);
    for (int i = 0; i < ki_length; i += PAGE_SIZE) {
        int len = (ki_length - i) < PAGE_SIZE ? (ki_length - i) : PAGE_SIZE;
        if (i2c_write_data(i, &buffer[i], len) != 0) {
            free(buffer);
            return 1;
        }
    }
    free(buffer);
    return 0;
}

int fill_eeprom(int ki_length) {
    unsigned char *buffer = malloc(ki_length);
    if (!buffer) return 1;
    memset(buffer, 1, ki_length);
    for (int i = 0; i < ki_length; i += PAGE_SIZE) {
        int len = (ki_length - i) < PAGE_SIZE ? (ki_length - i) : PAGE_SIZE;
        if (i2c_write_data(i, &buffer[i], len) != 0) {
            free(buffer);
            return 1;
        }
    }
    free(buffer);
    return 0;
}
