#include "DVA271_GPIO.h"
#include <stdio.h>
#include <stdlib.h>
#include "wiringPi.h"

// Define the GPIO pins used for the HC595
#define HC595_DATA_PIN 16  // Data pin (DS)
#define HC595_CLOCK_PIN 20 // Clock pin (SH_CP)
#define HC595_LATCH_PIN 21  // Latch pin (ST_CP)

// Function to initialize the HC595 shift register
int hc595_init() {
    // Initialize wiringPi
    if (wiringPiSetup() == -1) {
        return -1; // Return -1 if wiringPi setup fails
    }

    // Set the HC595 pins as output
    pinMode(HC595_DATA_PIN, OUTPUT);
    pinMode(HC595_CLOCK_PIN, OUTPUT);
    pinMode(HC595_LATCH_PIN, OUTPUT);

    // Optionally, set the initial state of the pins
    digitalWrite(HC595_DATA_PIN, LOW);
    digitalWrite(HC595_CLOCK_PIN, LOW);
    digitalWrite(HC595_LATCH_PIN, LOW);

    return 0; // Return 0 on success
}


// Function to initialize a GPIO pin
int gpio_init(int pin, bool output) {
    // Initialize wiringPi
    if (wiringPiSetup() == -1) {
        return -1; // Return -1 if wiringPi setup fails
    }

    // Set the pin mode based on the output parameter
    if (output) {
        pinMode(pin, OUTPUT); // Set pin as output
    } else {
        pinMode(pin, INPUT);  // Set pin as input
    }

    return 0; // Return 0 on success
}

// Function to flip the state of a digital pin
int flip_pin(int pin) {
    // Read the current state of the pin
    int pin_status = digitalRead(pin);
    
    // Flip the pin status
    pin_status = !pin_status; // Toggle the state (0 becomes 1, 1 becomes 0)

    // Write the new state back to the pin
    digitalWrite(pin, pin_status);

    return 0; // Return 0 on success
}

// Function to indicate temperature value using HC595
int temp_indicate() {
    // Initialize wiringPi
    if (wiringPiSetup() == -1) {
        return -1; // Return -1 if wiringPi setup fails
    }

    // Set the HC595 pins as output
    pinMode(HC595_DATA_PIN, OUTPUT);
    pinMode(HC595_CLOCK_PIN, OUTPUT);
    pinMode(HC595_LATCH_PIN, OUTPUT);

    // Read the temperature from CPU Core 3
    float temperature = read_cpu_temperature(3); // Read from Core 3
    if (temperature < 0) {
        return -1; // Return -1 if reading temperature fails
    }

    // Convert the temperature to a value suitable for the HC595
    uint8_t temp_value = (uint8_t)(temperature); // Truncate to fit in 8 bits

    // Set the latch pin low
    digitalWrite(HC595_LATCH_PIN, LOW);

    // Send the temperature value to the HC595
    for (int i = 0; i < 8; i++) {
        // Set the data pin
        digitalWrite(HC595_DATA_PIN, (temp_value & (1 << (7 - i))) ? HIGH : LOW);
        
        // Pulse the clock pin
        digitalWrite(HC595_CLOCK_PIN, HIGH);
        digitalWrite(HC595_CLOCK_PIN, LOW);
    }

    // Set the latch pin high to update the output
    digitalWrite(HC595_LATCH_PIN, HIGH);

    return 0; // Return 0 on success
}
