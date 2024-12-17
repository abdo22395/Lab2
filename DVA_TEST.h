#ifndef DVA_TEST_H
#define DVA_TEST_H

// Function prototypes
int init(int flag);         // Initialize necessary components with optional verbose output
int test_leds();           // Test program that blinks LEDs for 10 seconds
int test_hc595(int fake_temp); // Test program that sets the HC595 to indicate fake temperature
int test_write();          // Test program that writes a predetermined value to EEPROM
int test_read(); 


#endif // DVA_TEST_H