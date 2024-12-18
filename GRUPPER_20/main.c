#include "lib/DVA_TEST.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

// Funktion för att hantera avbrottssignaler
void cleanup(int signum) {
    printf("Avslutar programmet...\n");
    close_gpio_chip();
    exit(0);
}

int main(int argc, char *argv[]) {
    // Hantera signaler för korrekt stängning
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    int flag = 0;
    if (argc > 1 && strcmp(argv[1], "verbose") == 0) {
        flag = 1;
    }

    if (init(flag) != 0) {
        printf("Initialisering misslyckades.\n");
        return 1;
    }

    // Kör multithread-programmet
    if (multithread_fun() != 0) {
        printf("Multithread-programmet misslyckades.\n");
        return 1;
    }

    // Stäng GPIO-chip vid avslut
    close_gpio_chip();

    return 0;
}
