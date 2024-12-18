#include "DVA_TEST.h"
#include "DVA271_EEPROM.h"
#include "DVA271_GPIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define LED_PIN_23 23
#define LED_PIN_24 24


// Globala variabler och mutex
pthread_mutex_t eeprom_mutex;

// Funktion för att initiera och sätta mutex
int init(int flag) {
    int ret = eeprom_setup();
    if (flag) {
        if (ret == 0) printf("eeprom_setup returned SUCCESS\n");
        else printf("eeprom_setup returned %d\n", ret);
    }
    if (ret != 0) return ret;

    ret = gpio_init_chip();
    if (flag) {
        if (ret == 0) printf("gpio_init_chip returned SUCCESS\n");
        else printf("gpio_init_chip returned %d\n", ret);
    }
    if (ret != 0) return ret;

    ret = hc595_init();
    if (flag) {
        if (ret == 0) printf("hc595_init returned SUCCESS\n");
        else printf("hc595_init returned %d\n", ret);
    }
    if (ret != 0) return ret;

    // Initiera GPIO LEDs
    if (gpio_init(LED_PIN_23, true) != 0) {
        if (flag) printf("gpio_init for pin 23 failed\n");
        return 1;
    }
    if (gpio_init(LED_PIN_24, true) != 0) {
        if (flag) printf("gpio_init for pin 24 failed\n");
        return 1;
    }

    // Initiera mutex
    pthread_mutex_init(&eeprom_mutex, NULL);
    set_eeprom_mutex(&eeprom_mutex);

    return 0;
}

// Testa LEDs genom att blinka dem i 10 sekunder
int test_leds() {
    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        // Flip LED på GPIO 23
        flip_pin(LED_PIN_23);
        // Flip LED på GPIO 24
        flip_pin(LED_PIN_24);
        usleep(500000); // 500ms

        // Kontrollera tiden
        clock_gettime(CLOCK_MONOTONIC, &current);
        double elapsed = (current.tv_sec - start.tv_sec) +
                         (current.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= 10.0) break;
    }

    // Stäng av LEDs genom att sätta dem till låg värde
    if (set_pin_value(LED_PIN_23, 0) != 0) {
        perror("Failed to set GPIO 23 to low");
    }
    if (set_pin_value(LED_PIN_24, 0) != 0) {
        perror("Failed to set GPIO 24 to low");
    }

    return 0;
}

// Testa HC595 genom att sätta en fake temperatur
int test_hc595(int fake_temp) {
    int ret = temp_indicate((float)fake_temp);
    return ret;
}

// Testa skrivning till EEPROM
int test_write() {
    char test_joke[255] = "This is a test joke for EEPROM.\0";
    int joke_length = strlen(test_joke) + 1; // Inkludera null-terminator
    int ret = write_joke(test_joke, joke_length);
    return ret;
}

// Testa läsning från EEPROM
int test_read() {
    char *joke;
    int ret = get_joke(0, &joke);
    if (ret != 0) return ret;
    printf("Joke 0: %s\n", joke);
    free(joke);
    return 0;
}

// Thread-funktion för att kolla CPU temperatur
void *temp_thread(void *arg) {
    while (1) {
        // Läs CPU-temperatur från systemfilen
        FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        if (!fp) {
            perror("Failed to open temperature file");
            sleep(5);
            continue;
        }
        int temp_milli;
        fscanf(fp, "%d", &temp_milli);
        fclose(fp);
        float temp = temp_milli / 1000.0;

        // Indikera temperatur via HC595
        temp_indicate(temp);

        // Kontrollera om temp >= 85
        if (temp >= 85.0) {
            // Tänd alla LEDs
            pthread_mutex_lock(&eeprom_mutex);
            hc595_send_byte(0xFF);
            pthread_mutex_unlock(&eeprom_mutex);
        } else {
            // Stäng av alla LEDs
            pthread_mutex_lock(&eeprom_mutex);
            hc595_send_byte(0x00);
            pthread_mutex_unlock(&eeprom_mutex);
        }

        sleep(2); // Uppdatera var 2 sekund
    }
    return NULL;
}

// Thread-funktion för att skriva dåliga vitsar till EEPROM
void *write_joke_thread(void *arg) {
    const char *bad_jokes[] = {
        "Why did the chicken cross the road? To get to the other side!",
        "I would tell you a UDP joke, but you might not get it.",
        "Why do programmers prefer dark mode? Because light attracts bugs!",
        "Why was the math book sad? It had too many problems.",
        "I told my computer I needed a break, and it said no problem—it needed one too."
    };
    int num_jokes = sizeof(bad_jokes) / sizeof(bad_jokes[0]);
    int current_joke = 0;

    while (1) {
        pthread_mutex_lock(&eeprom_mutex);
        int ret = write_joke((char *)bad_jokes[current_joke], strlen(bad_jokes[current_joke]) + 1);
        pthread_mutex_unlock(&eeprom_mutex);
        if (ret != 0) {
            fprintf(stderr, "Failed to write joke: %d\n", ret);
        } else {
            printf("Wrote joke: %s\n", bad_jokes[current_joke]);
        }

        current_joke = (current_joke + 1) % num_jokes;
        sleep(10); // Write every 10 seconds
    }
    return NULL;
}

// Thread-funktion för att läsa vitsar från EEPROM och printa dem
void *read_joke_thread(void *arg) {
    int current_joke = 0;

    while (1) {
        char *joke;
        pthread_mutex_lock(&eeprom_mutex);
        int ret = get_joke(current_joke, &joke);
        pthread_mutex_unlock(&eeprom_mutex);
        if (ret != 0) {
            fprintf(stderr, "Failed to read joke %d: %d\n", current_joke, ret);
        } else {
            printf("Read joke %d: %s\n", current_joke, joke);
            free(joke);
        }

        current_joke = (current_joke + 1) % (EEPROM_SIZE / 256);
        sleep(15); // Read every 15 seconds
    }
    return NULL;
}

// Thread-funktion för att hantera LED-pinnar
void *led_thread(void *arg) {
    int pin = *(int *)arg;
    while (1) {
        flip_pin(pin);
        usleep(100000); // 100ms
    }
    return NULL;
}

// Multithread-programmet
int multithread_fun() {
    pthread_t threads[5];
    int led_pins[2] = {LED_PIN_23, LED_PIN_24};

    // Skapa trådar
    if (pthread_create(&threads[0], NULL, temp_thread, NULL) != 0) {
        perror("Failed to create temp_thread");
        return 1;
    }
    if (pthread_create(&threads[1], NULL, write_joke_thread, NULL) != 0) {
        perror("Failed to create write_joke_thread");
        return 1;
    }
    if (pthread_create(&threads[2], NULL, read_joke_thread, NULL) != 0) {
        perror("Failed to create read_joke_thread");
        return 1;
    }
    if (pthread_create(&threads[3], NULL, led_thread, &led_pins[0]) != 0) {
        perror("Failed to create led_thread for pin 23");
        return 1;
    }
    if (pthread_create(&threads[4], NULL, led_thread, &led_pins[1]) != 0) {
        perror("Failed to create led_thread for pin 24");
        return 1;
    }

    // Vänta på trådar (de kommer inte att avslutas)
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
