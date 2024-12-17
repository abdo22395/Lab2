#include "DVA271_TEST.h"
#include "DVA271_GPIO.h"
#include "DVA271_EEPROM.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

pthread_mutex_t eeprom_mutex;

void* monitor_cpu_temp(void* arg) {
    while (1) {
        int temp = 50 + (rand() % 40);  // Simulerad temperatur
        temp_indicate(temp);
        sleep(2);
    }
}

void* write_jokes(void* arg) {
    while (1) {
        pthread_mutex_lock(&eeprom_mutex);
        char joke[] = "Why do programmers prefer dark mode? Because light attracts bugs!";
        write_joke(joke, strlen(joke) + 1);
        pthread_mutex_unlock(&eeprom_mutex);
        sleep(5);
    }
}

void* read_jokes(void* arg) {
    while (1) {
        pthread_mutex_lock(&eeprom_mutex);
        char* joke;
        get_joke(0, &joke);
        printf("Read joke: %s\n", joke);
        pthread_mutex_unlock(&eeprom_mutex);
        sleep(5);
    }
}

void* blink_led(void* pin_ptr) {
    int pin = *(int*)pin_ptr;
    while (1) {
        flip_pin(pin);
        usleep(500000);
    }
}

int multithread_fun() {
    pthread_t temp_thread, write_thread, read_thread, led1_thread, led2_thread;
    int led1 = 23, led2 = 24;

    pthread_mutex_init(&eeprom_mutex, NULL);

    pthread_create(&temp_thread, NULL, monitor_cpu_temp, NULL);
    pthread_create(&write_thread, NULL, write_jokes, NULL);
    pthread_create(&read_thread, NULL, read_jokes, NULL);
    pthread_create(&led1_thread, NULL, blink_led, &led1);
    pthread_create(&led2_thread, NULL, blink_led, &led2);

    pthread_join(temp_thread, NULL);
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);
    pthread_join(led1_thread, NULL);
    pthread_join(led2_thread, NULL);

    pthread_mutex_destroy(&eeprom_mutex);
    return 0;
}
