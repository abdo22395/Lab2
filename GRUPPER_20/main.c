#include "lib/DVA_TEST.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
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

    return 0;
}
