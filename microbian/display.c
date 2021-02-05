// display.c
// Copyright (c) 2020 J. M. Spivey

#include "microbian.h"
#include "hardware.h"
#include <string.h>

/* Note that blank is not the same as an image that is all zeroes,
because it has the row bits set.  Copying blank and then setting
(actually, clearing) column bits for each row results in an image 
that displays properly. */

const image blank =
    IMAGE(0,0,0,0,0,
          0,0,0,0,0,
          0,0,0,0,0,
          0,0,0,0,0,
          0,0,0,0,0);

void image_clear(image img) {
    memcpy(img, blank, sizeof(image));
}

#ifdef UBIT_V1
#define PIX(i, j) (((i-1)<<5) | (j+3))

static unsigned img_map[5][5] = {
    { PIX(3,3), PIX(2,7), PIX(3,1), PIX(2,6), PIX(3,2) },
    { PIX(1,8), PIX(1,7), PIX(1,6), PIX(1,5), PIX(1,4) },
    { PIX(2,2), PIX(1,9), PIX(2,3), PIX(3,9), PIX(2,1) },
    { PIX(3,4), PIX(3,5), PIX(3,6), PIX(3,7), PIX(3,8) },
    { PIX(1,1), PIX(2,4), PIX(1,2), PIX(2,5), PIX(1,3) }
};

static unsigned map_pixel(int x, int y) {
    return img_map[y][x];
}
#endif

#ifdef UBIT_V2
static const unsigned col[] = {
    COL1, COL2, COL3, COL4, COL5
};

static unsigned map_pixel(int x, int y) {
    return ((4-y) << 6) | col[x];
}
#endif

void image_set(int x, int y, image img) {
    if (x < 0 || x >= 5 || y < 0 || y >= 5) return;
    unsigned p = map_pixel(x, y);
    CLR_BIT(img[p >> 5], p & 0x1f);
}


static image curimg; // A shared variable!

void display_task(int dummy) {
    int n = 0;

#ifdef UBIT_V1
    GPIO_DIRSET = LED_MASK;
    timer_pulse(3);
#endif
    
#ifdef UBIT_V2
    GPIO0_DIRSET = LED_MASK0;
    GPIO1_DIRSET  = LED_MASK1;
    
    // Set row pins to high-drive mode to increase brightness
    gpio_drive(ROW1, GPIO_DRIVE_S0H1);
    gpio_drive(ROW2, GPIO_DRIVE_S0H1);
    gpio_drive(ROW3, GPIO_DRIVE_S0H1);
    gpio_drive(ROW4, GPIO_DRIVE_S0H1);
    gpio_drive(ROW5, GPIO_DRIVE_S0H1);

    timer_pulse(5);
#endif

    image_clear(curimg);
    priority(P_HIGH);

    while (1) {
        // Carefully leave other bits unchanged
#ifdef UBIT_V1
        GPIO_OUTCLR = 0xfff0;
        GPIO_OUTSET = curimg[n++];
        if (n == 3) n = 0;
#endif
#ifdef UBIT_V2
        /* To avoid ghosting, clear GPIO0 (which contains the row bits)
           first and set it last. */
        GPIO0_OUTCLR = LED_MASK0;
        GPIO1_OUTCLR = LED_MASK1;
        GPIO1_OUTSET = curimg[n+1];
        GPIO0_OUTSET = curimg[n];
        n += 2;
        if (n == 10) n = 0;
#endif

        receive(PING, NULL);
    }
}

void display_show(const image img) {
    memcpy(curimg, img, sizeof(image));
}

void display_init(void) {
    start("Display", display_task, 0, STACK);
}
