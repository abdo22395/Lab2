#include "DVA_TEST.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "DVA271_GPIO.h" // Include your GPIO library
#include "DVA271_EEPROM.h" // Include the EEPROM header
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define HIGH 1
#define LOW 0

#define HC595_DATA_PIN 16
#define HC595_CLOCK_PIN 21
#define HC595_LATCH_PIN 20


// Function to initialize necessary components
int init(int flag) {
    int result;

    // Initialize GPIO pins
    result = gpio_init(0, true); // Initialize pin 0 as output
    if (result != 0) {
        if (flag == 1) {
            printf("gpio_init returned ERROR: Failed to initialize GPIO pin 0.\n");
        }
        return result; // Return error code
    }
    if (flag == 1) {
        printf("gpio_init returned SUCCESS.\n");
    }

    // Initialize other components as needed
    // For example, initializing the HC595 shift register
    result = hc595_init();
    if (result != 0) {
        if (flag == 1) {
            printf("hc595_init returned ERROR: Failed to initialize HC595.\n");
        }
        return result; // Return error code
    }
    if (flag == 1) {
        printf("hc595_init returned SUCCESS.\n");
    }

    return 0; // Return 0 on success
}

// Function to test LEDs by blinking them for 10 seconds
int test_leds() {
    // Define the GPIO  for the LED
    int led_ = 23; // Change this to the appropriate  number for your LED

    // Initialize the LED  as output
    gpio_init(led_, true);

    // Blink the LED for 10 seconds
    for (int i = 0; i < 10; i++) {
        digitalWrite(led_, HIGH); // Turn LED on
        usleep(500000); // Wait for 500 milliseconds
        digitalWrite(led_, LOW); // Turn LED off
        usleep(500000); // Wait for another 500 milliseconds
    }

    // Turn off the LED at the end
    digitalWrite(led_, LOW);

    return 0; // Return 0 on success
}

// Function to test HC595 by setting it to indicate a fake temperature
int test_hc595(int fake_temp) {
    // Ensure the fake temperature is within the valid range for an 8-bit value
    if (fake_temp < 0 || fake_temp > 255) {
        printf("ERROR: fake_temp must be between 0 and 255.\n");
        return -1; // Return error code for invalid temperature
    }

    // Initialize the HC595 s
    gpio_init(HC595_DATA_PIN, true);
    gpio_init(HC595_CLOCK_PIN, true);
    gpio_init(HC595_LATCH_PIN, true);

    // Set the latch pin low
    digitalWrite(HC595_LATCH_PIN, LOW);

    // Send the fake temperature value to the HC595
    for (int i = 0; i < 8; i++) {
        // Set the data pin
        digitalWrite(HC595_DATA_PIN, (fake_temp & (1 << (7 - i))) ? HIGH : LOW);
        
        // Pulse the clock pin
        digitalWrite(HC595_CLOCK_PIN, HIGH);
        digitalWrite(HC595_CLOCK_PIN, LOW);
    }

    // Set the latch pin high to update the output
    digitalWrite(HC595_LATCH_PIN, HIGH);

    printf("HC595 set to indicate fake temperature: %d\n", fake_temp);
    return 0; // Return 0 on success
}

// Function to test writing to EEPROM
void test_write() {
    // Example joke to write
    uint8_t joke_data[JOKE_LENGTH] = "Why did the bicycle fall over? Because it was two-tired!";
    int address = 0; // Starting address in EEPROM

    // Old write function call (to be updated)
    // old_eeprom_write(address, joke_data, strlen((char *)joke_data));

    // Updated write function call
    if (eeprom_write(address, joke_data, strlen((char *)joke_data)) == 0) {
        printf("Joke written successfully!\n");
    } else {
        printf("Failed to write joke to EEPROM.\n");
    }
}

void test_read() {
    // Buffer to hold the read joke
    uint8_t buffer[JOKE_LENGTH];
    int address = 0; // Starting address in EEPROM

    // Old read function call (to be updated)
    // old_eeprom_read(address, buffer, sizeof(buffer));

    // Updated read function call
    if (eeprom_read(address, buffer, sizeof(buffer)) == 0) {
        printf("Joke read from EEPROM: %s\n", buffer);
    } else {
        printf("Failed to read joke from EEPROM.\n");
    }
}
