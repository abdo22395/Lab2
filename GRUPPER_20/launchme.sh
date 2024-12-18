#!/bin/bash

# Kör programmet och bind det till CPU Core 3
taskset -c 3 ./dva271main &

echo "Programmet körs på CPU Core 3."
