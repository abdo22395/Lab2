#include "DVA271_EEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// Definiera EEPROM-adress och I2C-buss
#define EEPROM_I2C_ADDR 0x50 // Standardadress för 24LC64
#define I2C_BUS "/dev/i2c-1"

// Storlek på EEPROM (bytes)
#define EEPROM_SIZE 8192 // 8KB

static int file;
static pthread_mutex_t *eeprom_mutex_ptr;

// Initiera EEPROM och I2C
int eeprom_setup() {
    if ((file = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return 1;
    }
    if (ioctl(file, I2C_SLAVE, EEPROM_I2C_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave.");
        return 2;
    }
    return 0;
}

// Sätt mutex-pekare
void set_eeprom_mutex(pthread_mutex_t *mutex) {
    eeprom_mutex_ptr = mutex;
}

// Funktion för att läsa från EEPROM
int read_eeprom(int address, char *data, int length) {
    // Kontrollera adressgränser
    if (address + length > EEPROM_SIZE) {
        fprintf(stderr, "Read operation out of bounds\n");
        return 1;
    }

    // Skicka adressen
    char addr_buffer[2];
    addr_buffer[0] = (address >> 8) & 0xFF; // Höga byte
    addr_buffer[1] = address & 0xFF;        // Låga byte
    if (write(file, addr_buffer, 2) != 2) {
        perror("Failed to write EEPROM address");
        return 2;
    }

    // Läs data
    if (read(file, data, length) != length) {
        perror("Failed to read EEPROM data");
        return 3;
    }

    return 0;
}

// Funktion för att skriva till EEPROM
int write_eeprom(int address, const char *data, int length) {
    // Kontrollera adressgränser
    if (address + length > EEPROM_SIZE) {
        fprintf(stderr, "Write operation out of bounds\n");
        return 1;
    }

    // Förbered bufferten med adress och data
    char *buffer = malloc(length + 2);
    if (!buffer) {
        perror("Failed to allocate memory for write buffer");
        return 2;
    }
    buffer[0] = (address >> 8) & 0xFF; // Höga byte
    buffer[1] = address & 0xFF;        // Låga byte
    memcpy(buffer + 2, data, length);

    // Skriva till EEPROM
    if (write(file, buffer, length + 2) != length + 2) {
        perror("Failed to write EEPROM data");
        free(buffer);
        return 3;
    }

    free(buffer);

    // Vänta för skrivcykeln (t.ex., 5ms för 24LC64)
    usleep(5000);

    return 0;
}

// Hämta skämt nummer <number>
int get_joke(int number, char **ptr) {
    // Antag att varje skämt tar 256 bytes (255 för text + 1 för null-terminator)
    int joke_length = 255;
    int address = number * 256;

    char buffer[256];
    pthread_mutex_lock(eeprom_mutex_ptr);
    int ret = read_eeprom(address, buffer, joke_length);
    pthread_mutex_unlock(eeprom_mutex_ptr);

    if (ret != 0) {
        return ret;
    }

    // Allokera minne för skämtet
    *ptr = malloc(joke_length + 1);
    if (!*ptr) {
        perror("Failed to allocate memory for joke");
        return 4;
    }
    memcpy(*ptr, buffer, joke_length);
    (*ptr)[joke_length] = '\0'; // Null-terminera

    return 0;
}

// Skriv skämt till första lediga plats
int write_joke(char arr[255], int joke_length) {
    // Sök efter första lediga plats (all bytes = 0xFF)
    int address = 0;
    char buffer[255];
    while (address + joke_length <= EEPROM_SIZE) {
        pthread_mutex_lock(eeprom_mutex_ptr);
        int ret = read_eeprom(address, buffer, joke_length);
        pthread_mutex_unlock(eeprom_mutex_ptr);
        if (ret != 0) {
            return ret;
        }
        int empty = 1;
        for (int i = 0; i < joke_length; i++) {
            if (buffer[i] != 0xFF) {
                empty = 0;
                break;
            }
        }
        if (empty) {
            pthread_mutex_lock(eeprom_mutex_ptr);
            ret = write_eeprom(address, arr, joke_length);
            pthread_mutex_unlock(eeprom_mutex_ptr);
            return ret;
        }
        address += 256; // Hoppa till nästa skämtplats
    }
    fprintf(stderr, "No empty space found in EEPROM\n");
    return 5;
}

// Skriv skämt till specifik position
int write_joke_pos(char arr[255], int joke_length, int pos) {
    int address = pos * 256;
    if (address + joke_length > EEPROM_SIZE) {
        fprintf(stderr, "Write position out of bounds\n");
        return 1;
    }

    pthread_mutex_lock(eeprom_mutex_ptr);
    int ret = write_eeprom(address, arr, joke_length);
    pthread_mutex_unlock(eeprom_mutex_ptr);

    return ret;
}

// Rensa EEPROM (skriv 0 till alla platser upp till ki_length)
int clear_eeprom(int ki_length) {
    if (ki_length > EEPROM_SIZE) {
        fprintf(stderr, "Clear length exceeds EEPROM size\n");
        return 1;
    }

    char *buffer = calloc(1, ki_length);
    if (!buffer) {
        perror("Failed to allocate memory for clearing EEPROM");
        return 2;
    }

    pthread_mutex_lock(eeprom_mutex_ptr);
    int ret = write_eeprom(0, buffer, ki_length);
    pthread_mutex_unlock(eeprom_mutex_ptr);

    free(buffer);
    return ret;
}

// Fyll EEPROM (skriv 1 till alla platser upp till ki_length)
int fill_eeprom(int ki_length) {
    if (ki_length > EEPROM_SIZE) {
        fprintf(stderr, "Fill length exceeds EEPROM size\n");
        return 1;
    }

    char *buffer = malloc(ki_length);
    if (!buffer) {
        perror("Failed to allocate memory for filling EEPROM");
        return 2;
    }
    memset(buffer, 0xFF, ki_length); // 0xFF är all bits 1

    pthread_mutex_lock(eeprom_mutex_ptr);
    int ret = write_eeprom(0, buffer, ki_length);
    pthread_mutex_unlock(eeprom_mutex_ptr);

    free(buffer);
    return ret;
}
