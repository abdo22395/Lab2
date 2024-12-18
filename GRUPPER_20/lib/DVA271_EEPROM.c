// lib/DVA271_EEPROM.c

#include "DVA271_EEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define EEPROM_ADDRESS 0x50 // Justera efter din EEPROM-adress
#define EEPROM_SIZE 8192    // Exempelstorlek, justera efter din EEPROM
#define PAGE_SIZE 64        // Exempel sidstorlek, justera efter din EEPROM

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
    // EEPROM skrivtider
    usleep(50000); // 50 ms, justera enligt din EEPROM-spec
    return 0;
}

// Funktion för att läsa från I2C
static int i2c_read_data(unsigned short address, unsigned char *data, int length) {
    unsigned char addr_buffer[2];
    addr_buffer[0] = (address >> 8) & 0xFF;
    addr_buffer[1] = address & 0xFF;

    if (write(file, addr_buffer, 2) != 2) {
        perror("Failed to write address to the i2c bus");
        return -1;
    }

    if (read(file, data, length) != length) {
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
    unsigned short address = number * 255; // Antag att varje skämt tar 255 bytes
    unsigned char buffer[255];
    if (i2c_read_data(address, buffer, 255) != 0) {
        return -1;
    }
    // Allokera minne och kopiera skämtet
    *ptr = malloc(256);
    if (*ptr == NULL) return -1;
    strncpy(*ptr, (char*)buffer, 255);
    (*ptr)[255] = '\0';
    return 0;
}

int write_joke(char arr[255], int joke_length) {
    static int current_pos = 0;
    if (current_pos * 255 >= EEPROM_SIZE) {
        current_pos = 0; // Återställ till början
    }
    return write_joke_pos(arr, joke_length, current_pos++);
}

int write_joke_pos(char arr[255], int joke_length, int pos) {
    if (pos < 0 || pos * 255 >= EEPROM_SIZE) return 1;
    unsigned short address = pos * 255;
    return i2c_write_data(address, (unsigned char*)arr, joke_length);
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
