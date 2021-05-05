/* x3500/buggy.c */
/* Copyright (c) 2020 J. M. Spivey */

#include "microbian.h"

void pwm_change(int width0, int width1);
void pwm_init(void);


/* Main program */

#define REST 1500
#define SPEED 500

void drive(int lspeed, int rspeed)
{
    pwm_change(REST-lspeed, REST+rspeed);
}

void control_task(int dummy)
{
    timer_delay(1000);
    while (1) {
        drive(SPEED, SPEED);    /* Drive forward */
        timer_delay(1000);      /* ... for 1 second */
        drive(-SPEED, SPEED);   /* Turn left */
        timer_delay(1000);      /* ... for 1 second */
    }
}

void init(void)
{
    timer_init();
    pwm_init();
    start("Control", control_task, 0, STACK);
}
