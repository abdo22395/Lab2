#!/bin/bash
gcc main.c lib/DVA271_GPIO.c lib/DVA271_EEPROM.c lib/DVA271_TEST.c -o dva271main -lpthread
echo "Kompilering klar."
