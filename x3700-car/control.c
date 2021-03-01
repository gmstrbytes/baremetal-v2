// remote.c
// Copyright (c) 2020 J. M. Spivey

#include "microbian.h"
#include "hardware.h"
#include "lib.h"

static const unsigned left[] =
    IMAGE(0,0,1,0,0,
          0,1,1,1,1,
          1,1,1,1,1,
          0,1,1,1,1,
          0,0,1,0,0);

static const unsigned right[] =
    IMAGE(0,0,1,0,0,
          1,1,1,1,0,
          1,1,1,1,1,
          1,1,1,1,0,
          0,0,1,0,0);

static const unsigned ahead[] =
    IMAGE(0,0,1,0,0,
          0,1,1,1,0,
          1,1,1,1,1,
          0,1,1,1,0,
          0,1,1,1,0);

void sender_task(int dummy) {
    GPIO_PINCNF[BUTTON_A] = 0;
    GPIO_PINCNF[BUTTON_B] = 0;

    while (1) {
        int a = GET_BIT(GPIO_IN, BUTTON_A);
        int b = GET_BIT(GPIO_IN, BUTTON_B);

        if (a == 0 && b == 0) {
            display_show(ahead);
            radio_send("@B", 2);
        } else if (a == 0) {
            display_show(left);
            radio_send("@L", 2);
        } else if (b == 0) {
            display_show(right);
            radio_send("@R", 2);
        } else {
            display_show(blank);
        }

        timer_delay(100);
    }
}

void init(void) {
    radio_init();
    timer_init();
    display_init();
    start("Sender", sender_task, 0, STACK);
}
