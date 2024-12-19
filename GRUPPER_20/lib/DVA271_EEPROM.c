#include "DVA271_EEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>

static int file;

// Base address of the first EEPROM block (0x50)
#define EEPROM_BASE_ADDRESS 0x50

// Wait for the EEPROM write cycle to complete by polling for ACK
static int wait_for_eeprom(unsigned char chip_address) {
    // Set the device address and try zero-length writes until ACK received
    for (int i = 0; i < 100; i++) {
        if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
            perror("Failed to set I2C address for polling");
            return -1;
        }

        if (write(file, NULL, 0) >= 0) {
            // ACK means ready
            return 0;
        }
        usleep(5000); // 5ms delay
    }
    fprintf(stderr, "EEPROM did not become ready in time\n");
    return -1;
}

// Determine which chip and byte address within the chip for a given full address
static int select_chip_and_address(unsigned short full_address, unsigned char *chip_address, unsigned char *int_addr) {
    if (full_address >= EEPROM_SIZE) {
        fprintf(stderr, "Address out of EEPROM range\n");
        return -1;
    }

    int block = full_address / BLOCK_SIZE;
    *chip_address = EEPROM_BASE_ADDRESS + block;
    *int_addr = full_address % BLOCK_SIZE;
    return 0;
}

// Write data to EEPROM at a given absolute address (single-byte addressing)
static int i2c_write_data(unsigned short full_address, const unsigned char *data, int length) {
    unsigned char chip_address;
    unsigned char int_addr;
    if (select_chip_and_address(full_address, &chip_address, &int_addr) < 0) {
        return -1;
    }

    if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
        perror("Failed to set I2C slave address");
        return -1;
    }

    unsigned char buffer[1 + length];
    buffer[0] = int_addr; // single address byte
    memcpy(&buffer[1], data, length);

    if (write(file, buffer, 1 + length) != (1 + length)) {
        perror("Failed to write to the i2c bus");
        return -1;
    }

    // Wait for write cycle completion
    if (wait_for_eeprom(chip_address) != 0) {
        return -1;
    }

    return 0;
}

// Read data from EEPROM at a given absolute address (single-byte addressing)
static int i2c_read_data(unsigned short full_address, unsigned char *data, int length) {
    unsigned char chip_address;
    unsigned char int_addr;
    if (select_chip_and_address(full_address, &chip_address, &int_addr) < 0) {
        return -1;
    }

    // Set the slave address
    if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
        perror("Failed to set I2C slave address");
        return -1;
    }

    // Write the internal address pointer
    unsigned char addr_buf = int_addr;
    if (write(file, &addr_buf, 1) != 1) {
        perror("Failed to set EEPROM read address");
        return -1;
    }

    // Read data
    if (read(file, data, length) != length) {
        perror("Failed to read from EEPROM");
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
    // No WP handling, assume WP handled externally
    return 0;
}

int get_joke(int number, char **ptr) {
    if (number < 0) return -1;

    // Each "joke" is 255 bytes
    unsigned short address = number * 255;
    if (address + 255 > EEPROM_SIZE) {
        printf("Error: Address out of range\n");
        return -1;
    }

    unsigned char buffer[255];
    if (i2c_read_data(address, buffer, 255) != 0) {
        printf("Error: Failed to read the joke from EEPROM\n");
        return -1;
    }

    *ptr = malloc(256);
    if (*ptr == NULL) {
        printf("Error: Memory allocation failed\n");
        return -1;
    }
    memcpy(*ptr, buffer, 255);
    (*ptr)[255] = '\0';

    return 0; // Success
}

int write_joke(char arr[255], int joke_length) {
    if (joke_length > 255) {
        printf("Error: joke_length exceeds buffer size of 255.\n");
        return 1;
    }

    memset(arr + joke_length, 0, 255 - joke_length);

    // Write at position 0
    return write_joke_pos(arr, 255, 0);
}

int write_joke_pos(char arr[255], int joke_length, int pos) {
    if (pos < 0) {
        printf("Error: Invalid position (negative)\n");
        return 1;
    }

    unsigned short address = pos * 255;
    if (address + joke_length > EEPROM_SIZE) {
        printf("Error: Address out of EEPROM range\n");
        return 1;
    }

    int remaining = joke_length;
    int offset = 0;

    while (remaining > 0) {
        int page_offset = address % PAGE_SIZE;
        int space_in_page = PAGE_SIZE - page_offset;
        int write_length = (remaining < space_in_page) ? remaining : space_in_page;

        if (i2c_write_data(address, (unsigned char*)&arr[offset], write_length) != 0) {
            printf("Error: Failed to write to EEPROM at address 0x%04X\n", address);
            return 1;
        }

        address += write_length;
        offset += write_length;
        remaining -= write_length;
    }

    return 0; // Success
}
