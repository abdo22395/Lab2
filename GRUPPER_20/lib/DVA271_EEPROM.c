#include "DVA271_EEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>

static int file;

// Polls the EEPROM until it is ready after a write
// Returns 0 when ready, -1 on timeout
static int wait_for_eeprom() {
    // The EEPROM will not ACK if it's still busy with the internal write cycle.
    // Here we send a zero-length write just to probe for ACK.
    for (int i = 0; i < 100; i++) { // Attempt up to 100 times
        if (write(file, NULL, 0) >= 0) {
            // ACK received, EEPROM is ready
            return 0;
        }
        usleep(5000); // Wait 5ms before trying again
    }
    fprintf(stderr, "EEPROM did not become ready in time\n");
    return -1;
}

// Write data to I2C EEPROM at a given address
// This function writes up to one page at a time
static int i2c_write_data(unsigned short address, const unsigned char *data, int length) {
    unsigned char buffer[2 + length];
    buffer[0] = (address >> 8) & 0xFF; // High address byte
    buffer[1] = address & 0xFF;        // Low address byte
    memcpy(&buffer[2], data, length);

    if (write(file, buffer, 2 + length) != (2 + length)) {
        perror("Failed to write to the i2c bus");
        return -1;
    }

    // After writing, we must wait for the EEPROM to finish its internal write cycle
    if (wait_for_eeprom() != 0) {
        return -1;
    }

    return 0;
}

// Read data from I2C EEPROM using a repeated start
static int i2c_read_data(unsigned short address, unsigned char *data, int length) {
    unsigned char addr_buffer[2];
    addr_buffer[0] = (address >> 8) & 0xFF;
    addr_buffer[1] = address & 0xFF;

    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    messages[0].addr  = EEPROM_ADDRESS;
    messages[0].flags = 0;             // Write address
    messages[0].len   = 2;
    messages[0].buf   = addr_buffer;

    messages[1].addr  = EEPROM_ADDRESS;
    messages[1].flags = I2C_M_RD;      // Read data
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

    if (wp_init() != 0) {
        printf("Failed to initialize WP pin\n");
        return 3;
    }

    // Enable write protection by default
    set_wp(true);
    return 0;
}

int get_joke(int number, char **ptr) {
    if (number < 0) return -1;

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

    // Allocate and copy, ensuring null termination
    *ptr = malloc(256);
    if (*ptr == NULL) {
        printf("Error: Memory allocation failed\n");
        return -1;
    }
    memcpy(*ptr, buffer, 255);
    (*ptr)[255] = '\0';

    return 0; // Success
}

// Writes a full 255-byte block for the joke, zero-padding the rest
int write_joke(char arr[255], int joke_length) {
    if (joke_length > 255) {
        printf("Error: joke_length exceeds buffer size of 255.\n");
        return 1;
    }

    // Zero out the remainder
    memset(arr + joke_length, 0, 255 - joke_length);

    // Disable write protection
    set_wp(false);
    int result = write_joke_pos(arr, 255, 0);
    // Re-enable write protection
    set_wp(true);

    return result;
}

// Writes 'joke_length' bytes at position pos * 255, handling page boundaries
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
        // Calculate how many bytes we can write on this page
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

int clear_eeprom(int ki_length) {
    if (ki_length > EEPROM_SIZE) ki_length = EEPROM_SIZE;
    unsigned char *buffer = malloc(ki_length);
    if (!buffer) return 1;
    memset(buffer, 0, ki_length);

    set_wp(false);
    int result = 0;
    int remaining = ki_length;
    int address = 0;

    while (remaining > 0) {
        int page_offset = address % PAGE_SIZE;
        int space_in_page = PAGE_SIZE - page_offset;
        int write_length = (remaining < space_in_page) ? remaining : space_in_page;

        if (i2c_write_data(address, &buffer[address], write_length) != 0) {
            result = 1;
            break;
        }

        address += write_length;
        remaining -= write_length;
    }

    set_wp(true);
    free(buffer);
    return result;
}

int fill_eeprom(int ki_length) {
    if (ki_length > EEPROM_SIZE) ki_length = EEPROM_SIZE;
    unsigned char *buffer = malloc(ki_length);
    if (!buffer) return 1;
    memset(buffer, 1, ki_length);

    set_wp(false);
    int result = 0;
    int remaining = ki_length;
    int address = 0;

    while (remaining > 0) {
        int page_offset = address % PAGE_SIZE;
        int space_in_page = PAGE_SIZE - page_offset;
        int write_length = (remaining < space_in_page) ? remaining : space_in_page;

        if (i2c_write_data(address, &buffer[address], write_length) != 0) {
            result = 1;
            break;
        }

        address += write_length;
        remaining -= write_length;
    }

    set_wp(true);
    free(buffer);
    return result;
}
