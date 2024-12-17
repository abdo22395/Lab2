#include "DVA271_CPU.h"
#include <stdio.h>
#include <stdlib.h>

// Function to read the temperature from a specified CPU core
float read_cpu_temperature(int core) {
    FILE *fp;
    float temperature = 0.0;
    char path[50];

    // Construct the file path for the specified CPU core
    snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/temp", core);

    // Open the temperature file for the specified CPU core
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Failed to read temperature");
        return -1; // Return -1 on error
    }

    // Read the temperature value
    fscanf(fp, "%f", &temperature);
    fclose(fp);

    // Convert from millidegrees Celsius to degrees Celsius
    temperature /= 1000.0;

    return temperature;
}