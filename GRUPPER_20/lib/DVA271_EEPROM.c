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

// Each block is 8KB for a total of 32KB across addresses 0x50 - 0x53
#define BLOCK_SIZE 8192
#define EEPROM_BASE_ADDRESS 0x50

// Polls the EEPROM until it is ready after a write
// Returns 0 when ready, -1 on timeout
static int wait_for_eeprom(unsigned char chip_address) {
    for (int i = 0; i < 100; i++) {
        // Set the slave address again for each poll
        if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
            perror("Failed to set I2C address for polling");
            return -1;
        }

        // A zero-length write tests for ACK
        if (write(file, NULL, 0) >= 0) {
            return 0; // Acknowledged, EEPROM ready
        }
        usleep(5000); // Wait 5ms before retry
    }
    fprintf(stderr, "EEPROM did not become ready in time\n");
    return -1;
}

// Compute which chip and internal address to use based on full memory address
static int select_chip_and_address(unsigned short full_address, unsigned char *chip_address, unsigned short *int_addr) {
    if (full_address >= EEPROM_SIZE) {
        fprintf(stderr, "Address out of EEPROM range\n");
        return -1;
    }

    int block = full_address / BLOCK_SIZE;
    *chip_address = EEPROM_BASE_ADDRESS + block;
    *int_addr = full_address % BLOCK_SIZE;
    return 0;
}

// Write data to the EEPROM at a given absolute address (handle multiple chips)
static int i2c_write_data(unsigned short full_address, const unsigned char *data, int length) {
    unsigned char chip_address;
    unsigned short int_addr;
    if (select_chip_and_address(full_address, &chip_address, &int_addr) < 0) {
        return -1;
    }

    // Set the slave address for this transaction
    if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
        perror("Failed to set I2C slave address");
        return -1;
    }

    unsigned char buffer[2 + length];
    buffer[0] = (int_addr >> 8) & 0xFF;
    buffer[1] = int_addr & 0xFF;
    memcpy(&buffer[2], data, length);

    if (write(file, buffer, 2 + length) != (2 + length)) {
        perror("Failed to write to the i2c bus");
        return -1;
    }

    // Wait for internal write cycle
    if (wait_for_eeprom(chip_address) != 0) {
        return -1;
    }

    return 0;
}

// Read data from EEPROM (with repeated start) at a given absolute address
static int i2c_read_data(unsigned short full_address, unsigned char *data, int length) {
    unsigned char chip_address;
    unsigned short int_addr;
    if (select_chip_and_address(full_address, &chip_address, &int_addr) < 0) {
        return -1;
    }

    // Set the slave address
    if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
        perror("Failed to set I2C slave address");
        return -1;
    }

    unsigned char addr_buffer[2];
    addr_buffer[0] = (int_addr >> 8) & 0xFF;
    addr_buffer[1] = int_addr & 0xFF;

    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    messages[0].addr  = chip_address;
    messages[0].flags = 0;             // Write address
    messages[0].len   = 2;
    messages[0].buf   = addr_buffer;

    messages[1].addr  = chip_address;
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

    // Since we are no longer handling WP in software, nothing else needed here
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

    // WP handling removed - ensure WP is disabled externally before calling
    int result = write_joke_pos(arr, 255, 0);
    return result;
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
        int page_offset = (address % PAGE_SIZE);
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

    // WP handling removed - ensure WP is disabled externally
    int result = 0;
    int remaining = ki_length;
    unsigned short address = 0;

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

    free(buffer);
    return result;
}

int fill_eeprom(int ki_length) {
    if (ki_length > EEPROM_SIZE) ki_length = EEPROM_SIZE;
    unsigned char *buffer = malloc(ki_length);
    if (!buffer) return 1;
    memset(buffer, 1, ki_length);

    // WP handling removed - ensure WP is disabled externally
    int result = 0;
    int remaining = ki_length;
    unsigned short address = 0;

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

    free(buffer);
    return result;
}
