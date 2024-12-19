#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "lib/DVA271_EEPROM.h"
#include "lib/DVA271_GPIO.h"
#include "lib/DVA_TEST.h"

// Mutex for EEPROM access to avoid conflicts between read/write threads
pthread_mutex_t eeprom_mutex;

// Thread that continuously writes jokes to the EEPROM
void* write_jokes_thread(void* arg) {
    // A set of jokes to write into the EEPROM in a loop
    const char *jokes[] = {
        "Why did the programmer quit his job? Because he didn't get arrays!",
        "There are 10 types of people in the world: those who understand binary, and those who don't.",
        "A SQL query goes into a bar, walks up to two tables and asks, 'Can I join you?'",
        "Why do programmers always mix up Halloween and Christmas? Because Oct 31 == Dec 25!"
    };
    int num_jokes = sizeof(jokes) / sizeof(jokes[0]);
    int joke_index = 0;

    while (1) {
        // Lock the EEPROM mutex before write operations
        pthread_mutex_lock(&eeprom_mutex);

        const char *current_joke = jokes[joke_index];
        char arr[255];
        memset(arr, 0, sizeof(arr));
        size_t joke_len = strlen(current_joke);
        if (joke_len > 255) {
            joke_len = 255; // Truncate if too long
        }
        memcpy(arr, current_joke, joke_len);

        // Write the chosen joke to the EEPROM at position 0
        if (write_joke(arr, (int)joke_len) != 0) {
            printf("Misslyckades att skriva skämt till EEPROM\n");
        }

        // Unlock the mutex after the write is done
        pthread_mutex_unlock(&eeprom_mutex);

        // Move to the next joke in the list (wrap around)
        joke_index = (joke_index + 1) % num_jokes;

        // Sleep for 2 seconds before writing the next joke
        sleep(2);
    }
    return NULL;
}

// Thread that continuously reads the first joke from the EEPROM and prints it
void* read_jokes_thread(void* arg) {
    while (1) {
        // Lock the EEPROM mutex before read operations
        pthread_mutex_lock(&eeprom_mutex);

        char* joke;
        // Attempt to read joke at position 0
        if (get_joke(0, &joke) == 0) { 
            printf("Läst skämt: %s\n", joke);
            free(joke); // Free the allocated memory after printing
        } else {
            printf("Ingen skämt hittades eller läsfel\n");
        }

        // Unlock the mutex after reading
        pthread_mutex_unlock(&eeprom_mutex);

        // Sleep for 2 seconds before reading again
        sleep(2);
    }
    return NULL;
}

// Thread to blink a specific LED at a fixed interval
void* led_blink_thread(void* arg) {
    int pin = *(int*)arg;
    while (1) {
        flip_pin(pin);
        usleep(100000); // Blink every 100 ms
    }
    return NULL;
}

int main() {
    // Initialize mutex for EEPROM operations
    if (pthread_mutex_init(&eeprom_mutex, NULL) != 0) {
        printf("Mutex init misslyckades\n");
        return 1;
    }

    // Initialize the EEPROM (open I2C device)
    if (eeprom_setup() != 0) {
        printf("EEPROM setup misslyckades\n");
        return 1;
    }

    // Initialize GPIO and shift register lines
    if (hc595_init() != 0) {
        printf("HC595 init misslyckades\n");
        return 1;
    }

    // Create threads:
    // - One that writes jokes to EEPROM continuously
    // - One that reads jokes from EEPROM continuously
    // - Two that blink separate LEDs
    pthread_t write_thread, read_thread, led1_thread, led2_thread;

    if (pthread_create(&write_thread, NULL, write_jokes_thread, NULL) != 0) {
        printf("Misslyckades att skapa write_jokes_thread\n");
        return 1;
    }

    if (pthread_create(&read_thread, NULL, read_jokes_thread, NULL) != 0) {
        printf("Misslyckades att skapa read_jokes_thread\n");
        return 1;
    }

    int led1 = 23, led2 = 24;
    if (pthread_create(&led1_thread, NULL, led_blink_thread, &led1) != 0) {
        printf("Misslyckades att skapa led1_thread\n");
        return 1;
    }

    if (pthread_create(&led2_thread, NULL, led_blink_thread, &led2) != 0) {
        printf("Misslyckades att skapa led2_thread\n");
        return 1;
    }

    // The threads run indefinitely, but we join them here to keep the main thread alive
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);
    pthread_join(led1_thread, NULL);
    pthread_join(led2_thread, NULL);

    // Destroy the mutex before exiting
    pthread_mutex_destroy(&eeprom_mutex);

    return 0;
}
