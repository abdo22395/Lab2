#include "lib/DVA_TEST.h"
#include "DVA271_EEPROM.h"
#include "DVA271_GPIO.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

// Definiera GPIO-pinnar för LEDs
#define LED_PIN_23 23
#define LED_PIN_24 24

// Funktioner för att testa varje modul
void test_eeprom();
void test_gpio();
void test_hc595();
void test_multithreading();

// Funktion för att hantera signaler för korrekt avslutning
void cleanup(int signum) {
    printf("\nAvslutar programmet...\n");
    close_gpio_chip();
    exit(0);
}

int main(int argc, char *argv[]) {
    // Hantera signaler för korrekt avslutning
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    int flag = 0;
    if (argc > 1 && strcmp(argv[1], "verbose") == 0) {
        flag = 1;
    }

    // Initiera systemet
    printf("Initierar systemet...\n");
    if (init(flag) != 0) {
        printf("Initialisering misslyckades.\n");
        return 1;
    }
    printf("Initialisering lyckades.\n");

    // Testa EEPROM
    printf("\n--- Testar EEPROM ---\n");
    test_eeprom();

    // Testa GPIO
    printf("\n--- Testar GPIO ---\n");
    test_gpio();

    // Testa HC595
    printf("\n--- Testar HC595 ---\n");
    test_hc595();

    // Testa Multitrådning (valfritt)
    printf("\n--- Testar Multitrådning ---\n");
    // Om du vill testa multitrådning, avkommentera följande rad:
    // test_multithreading();

    // Avsluta programmet
    printf("\nAlla tester utförda.\n");

    // Stäng GPIO-chipet
    close_gpio_chip();

    return 0;
}

void test_eeprom() {
    // Skriv ett testskämt till EEPROM
    char test_joke[255] = "Varför korsade kycklingen vägen? För att komma till andra sidan!";
    int joke_length = strlen(test_joke) + 1; // Inkludera null-terminator

    printf("Skriver skämt till EEPROM: \"%s\"\n", test_joke);
    if (write_joke(test_joke, joke_length) != 0) {
        printf("EEPROM skrivning misslyckades.\n");
    } else {
        printf("EEPROM skrivning lyckades.\n");
    }

    // Läs skämtet tillbaka från EEPROM
    char *read_joke = NULL;
    if (get_joke(0, &read_joke) != 0) {
        printf("EEPROM läsning misslyckades.\n");
    } else {
        printf("EEPROM läst skämt: \"%s\"\n", read_joke);
        free(read_joke);
    }
}

void test_gpio() {
    // Flippa LED-pinnar 23 och 24
    printf("Flippar LED pin %d.\n", LED_PIN_23);
    if (flip_pin(LED_PIN_23) != 0) {
        printf("Misslyckades att flippa LED pin %d.\n", LED_PIN_23);
    } else {
        printf("LED pin %d flippad.\n", LED_PIN_23);
    }

    printf("Flippar LED pin %d.\n", LED_PIN_24);
    if (flip_pin(LED_PIN_24) != 0) {
        printf("Misslyckades att flippa LED pin %d.\n", LED_PIN_24);
    } else {
        printf("LED pin %d flippad.\n", LED_PIN_24);
    }

    // Sätt LED-pinnar till HIGH
    printf("Sätter LED pin %d till HIGH.\n", LED_PIN_23);
    if (set_pin_value(LED_PIN_23, 1) != 0) {
        printf("Misslyckades att sätta LED pin %d till HIGH.\n", LED_PIN_23);
    } else {
        printf("LED pin %d satt till HIGH.\n", LED_PIN_23);
    }

    printf("Sätter LED pin %d till HIGH.\n", LED_PIN_24);
    if (set_pin_value(LED_PIN_24, 1) != 0) {
        printf("Misslyckades att sätta LED pin %d till HIGH.\n", LED_PIN_24);
    } else {
        printf("LED pin %d satt till HIGH.\n", LED_PIN_24);
    }

    // Sätt LED-pinnar till LOW
    printf("Sätter LED pin %d till LOW.\n", LED_PIN_23);
    if (set_pin_value(LED_PIN_23, 0) != 0) {
        printf("Misslyckades att sätta LED pin %d till LOW.\n", LED_PIN_23);
    } else {
        printf("LED pin %d satt till LOW.\n", LED_PIN_23);
    }

    printf("Sätter LED pin %d till LOW.\n", LED_PIN_24);
    if (set_pin_value(LED_PIN_24, 0) != 0) {
        printf("Misslyckades att sätta LED pin %d till LOW.\n", LED_PIN_24);
    } else {
        printf("LED pin %d satt till LOW.\n", LED_PIN_24);
    }
}

void test_hc595() {
    // Skicka ett testbyte till HC595
    unsigned char test_byte = 0xAA; // Binär 10101010
    printf("Skickar testbyte 0x%X till HC595.\n", test_byte);
    if (hc595_send_byte(test_byte) != 0) {
        printf("Misslyckades att skicka byte till HC595.\n");
    } else {
        printf("Byte 0x%X skickat till HC595.\n", test_byte);
    }

    // Simulera en hög temperatur och indikera den
    float test_temp = 90.0; // Celsius
    printf("Indikerar temperatur: %.1f°C\n", test_temp);
    if (temp_indicate(test_temp) != 0) {
        printf("Misslyckades att indikera temperatur via HC595.\n");
    } else {
        printf("Temperatur indikerad via HC595.\n");
    }
}

void test_multithreading() {
    printf("Initierar multitrådningstest...\n");

    // Skapa och starta trådar
    pthread_t threads[5];
    int led_pins[2] = {LED_PIN_23, LED_PIN_24};

    if (pthread_create(&threads[0], NULL, temp_thread, NULL) != 0) {
        perror("Misslyckades att skapa temp_thread");
    }
    if (pthread_create(&threads[1], NULL, write_joke_thread, NULL) != 0) {
        perror("Misslyckades att skapa write_joke_thread");
    }
    if (pthread_create(&threads[2], NULL, read_joke_thread, NULL) != 0) {
        perror("Misslyckades att skapa read_joke_thread");
    }
    if (pthread_create(&threads[3], NULL, led_thread, &led_pins[0]) != 0) {
        perror("Misslyckades att skapa led_thread för pin 23");
    }
    if (pthread_create(&threads[4], NULL, led_thread, &led_pins[1]) != 0) {
        perror("Misslyckades att skapa led_thread för pin 24");
    }

    // Vänta på att trådarna ska avslutas (vilket de inte gör i detta test)
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Multitrådningstest avslutat.\n");
}
