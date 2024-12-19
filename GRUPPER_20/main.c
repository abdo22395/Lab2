// main.c

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "lib/DVA271_EEPROM.h"
#include "lib/DVA271_GPIO.h"
#include "lib/DVA_TEST.h"

// Mutex för EEPROM-åtkomst
pthread_mutex_t eeprom_mutex;

void* write_jokes_thread(void* arg) {
    // An array of jokes to alternate between
    const char *jokes[] = {
        "Why did the programmer quit his job? Because he didn't get arrays!",
        "There are 10 types of people in the world: those who understand binary, and those who don't.",
        "A SQL query goes into a bar, walks up to two tables and asks, 'Can I join you?'",
        "Why do programmers always mix up Halloween and Christmas? Because Oct 31 == Dec 25!"
    };
    int num_jokes = sizeof(jokes) / sizeof(jokes[0]);

    // Keep track of which joke to write each time
    int joke_index = 0;

    while (1) {
        pthread_mutex_lock(&eeprom_mutex);

        // Select the current joke
        const char *current_joke = jokes[joke_index];

        // Copy the joke into the buffer and pad with zeros if needed
        char arr[255];
        memset(arr, 0, sizeof(arr));
        size_t joke_len = strlen(current_joke);
        if (joke_len > 255) {
            joke_len = 255; // Truncate if too long
        }
        memcpy(arr, current_joke, joke_len);

        // Write the joke to the EEPROM
        if (write_joke(arr, (int)joke_len) != 0) {
            printf("Misslyckades att skriva skämt till EEPROM\n");
        } else {
            printf("Skämt skrivet: %s\n", current_joke);
        }

        pthread_mutex_unlock(&eeprom_mutex);

        // Move to the next joke
        joke_index = (joke_index + 1) % num_jokes;

        sleep(1); // Justera efter behov
    }
    return NULL;
}
void* read_jokes_thread(void* arg) {
    while (1) {
         
        pthread_mutex_lock(&eeprom_mutex);
        char* joke;
        if (get_joke(0, &joke) == 0) { 
            printf("Läst skämt: %s\n", joke);

        } else {
            printf("Ingen skämt hittades eller läsfel\n");
        }
        pthread_mutex_unlock(&eeprom_mutex);
        // Justera efter behov
        sleep(1);
    }
    return NULL;
}

void* led_blink_thread(void* arg) {
    int pin = *(int*)arg;
    while (1) {
        flip_pin(pin);
        usleep(100000); // 100 ms
    }
    return NULL;
}

int main() {
    // Initialisera mutex
    if (pthread_mutex_init(&eeprom_mutex, NULL) != 0) {
        printf("Mutex init misslyckades\n");
        return 1;
    }



    // Initialisera hårdvara
    if (eeprom_setup() != 0) {
        printf("EEPROM setup misslyckades\n");
        return 1;
    }

     if (hc595_init() != 0) {
        printf("HC595 init misslyckades\n");
        return 1;
    }

    // Skapa trådar
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

    // Vänta på trådarna (de körs i oändlighet)
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);
    pthread_join(led1_thread, NULL);
    pthread_join(led2_thread, NULL);

    // Destruktera mutex
    pthread_mutex_destroy(&eeprom_mutex);
    return 0;
}
