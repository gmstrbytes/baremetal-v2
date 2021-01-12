// level.c
// Copyright (c) 2019 J. M. Spivey

#include "microbian.h"
#include "hardware.h"
#include "lib.h"
#include "accel.h"

/* light -- show one pixel */
void light(int x, int y) {
    image screen;
    image_clear(screen);
    image_set(x, y, screen);
    display_show(screen);
}

/* scale -- map acceleration to coordinate */
static int scale(int x) {
    if (x < -20) return 4;
    if (x < -10) return 3;
    if (x <= 10) return 2;
    if (x <= 20) return 1;
    return 0;
}

/* main_task -- show the spirit level */
static void main(int n) {
    int x, y, z;

    printf("Hello\n\n");
    accel_start();
    while (1) {
        timer_delay(200);
        accel_reading(&x, &y, &z);
        printf("x=%d y=%d z=%d\n", x, y, z);
        x = scale(x); y = scale(y);
        light(x, y);
    }
}

void init(void) {
    serial_init();
    timer_init();
    i2c_init(0);
    display_init();
    start("Main", main, 0, STACK);
}
