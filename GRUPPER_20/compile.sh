#!/bin/bash

# Kompilera alla C-filer och länka dem tillsammans
gcc -o dva271main main.c lib/DVA271_EEPROM.c lib/DVA271_GPIO.c lib/DVA_TEST.c -lpthread -lgpiod

if [ $? -eq 0 ]; then
    echo "Kompilering lyckades."
else
    echo "Kompilering misslyckades."
    exit 1
fi
