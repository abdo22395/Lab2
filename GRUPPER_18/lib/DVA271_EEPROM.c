#include "DVA271_EEPROM.h"

#define EEPROM_SIZE 32768  // 32 KiB EEPROM
#define BLOCK_SIZE 255

static char eeprom[EEPROM_SIZE];

int eeprom_setup() {
    memset(eeprom, 0, EEPROM_SIZE);
    printf("EEPROM initialized.\n");
    return 0;
}

int get_joke(int number, char **ptr) {
    int pos = number * BLOCK_SIZE;
    if (pos >= EEPROM_SIZE || eeprom[pos] == '\0') {
        printf("No joke found at block %d.\n", number);
        return 1;
    }
    *ptr = &eeprom[pos];
    return 0;
}

int write_joke(char arr[255], int joke_length) {
    for (int i = 0; i < EEPROM_SIZE / BLOCK_SIZE; i++) {
        if (eeprom[i * BLOCK_SIZE] == '\0') {
            memcpy(&eeprom[i * BLOCK_SIZE], arr, joke_length);
            printf("Joke written at block %d.\n", i);
            return 0;
        }
    }
    return 1;
}

int write_joke_pos(char arr[255], int joke_length, int pos) {
    int location = pos * BLOCK_SIZE;
    if (location >= EEPROM_SIZE) return 1;
    memcpy(&eeprom[location], arr, joke_length);
    printf("Joke written at block %d.\n", pos);
    return 0;
}

int clear_eeprom(int ki_length) {
    memset(eeprom, 0, ki_length * 1024);
    printf("EEPROM cleared up to %d KiB.\n", ki_length);
    return 0;
}

int fill_eeprom(int ki_length) {
    memset(eeprom, 1, ki_length * 1024);
    printf("EEPROM filled up to %d KiB.\n", ki_length);
    return 0;
}
