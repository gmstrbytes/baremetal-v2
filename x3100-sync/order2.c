// x3100/order.c
// Copyright (c) 2018-20 J. M. Spivey

#include "microbian.h"
#include "lib.h"
#include <string.h>

int MAY, FARAGE, HUMPHRYS;

#define RELEASE 23

char *slogan[] = {
    "no deal is better than a bad deal",
    "BREXIT MEANS BREXIT!"
};

void host(int n) {
    while (1) {
        receive(REQUEST, NULL);
        receive(RELEASE, NULL);
    }
}

void guest(int n) {
    while (1) {
        send(HUMPHRYS, REQUEST, NULL);
        printf("%s\n", slogan[n]);
        send(HUMPHRYS, RELEASE, NULL);
    }
}

void init(void) {
    serial_init();
    MAY = start("May", guest, 0, STACK);
    FARAGE = start("Farage", guest, 1, STACK);
    HUMPHRYS = start("Humphrys", host, 0, STACK);
}
