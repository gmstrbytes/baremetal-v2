// bitbang.c
// Copyright (c) 2020 J. M. Spivey

#include "hardware.h"

/* Square wave outputs on pads 0 (bit-banged in software) and 1
   (hardware control).  Meanwhile, the random number generator is
   causing interrupts at irregular intervals, with a pulse on pad 2. */

/* rng_init -- initialise hardware random number generator */
void rng_init(void) {
    /* Setting the DERCEN bit enables the bias elimination algorithm,
       and makes the intervals between random bytes irregular. */
    SET_BIT(RNG_CONFIG, RNG_CONFIG_DERCEN);
    RNG_VALRDY = 0;
    RNG_INTENSET = BIT(RNG_INT_VALRDY);
    enable_irq(RNG_IRQ);
    RNG_START = 1;
}

/* rng_handler -- interrupt handler for random number generator */
void rng_handler(void) {
    if (RNG_VALRDY) {
        gpio_out(PAD2, 1);
        RNG_VALRDY = 0;         // Just acknowledge the interrupt
        gpio_out(PAD2, 0);
    }
}

/* sqwave_init -- set up high-frequency square wave on pad 1 */
void sqwave_init(void) {
    /* Set up timer 2 to count at 16Mhz, and link it via the 'programmable
       peripheral interconnect' to a task that toggles the GPIO pin on each
       compare event. */

    // Set up channel 0 of the GPIO-tasks-and-events system to toggle the pin
    GPIOTE_CONFIG[0] =
        FIELD(GPIOTE_CONFIG_MODE, GPIOTE_MODE_Task)
        | FIELD(GPIOTE_CONFIG_PSEL, PAD1)
        | FIELD(GPIOTE_CONFIG_POLARITY, GPIOTE_POLARITY_Toggle);

    // Use channel 0 of the PPI to activate the GPIO task on each timer event
    PPI_CH[0].EEP = &TIMER2_COMPARE[0];
    PPI_CH[0].TEP = &GPIOTE_OUT[0];
    PPI_CHENSET = BIT(0);

    // Timer 2 counts repeatedly at 16MHz, resetting once every 8 cycles
    TIMER2_MODE = TIMER_MODE_Timer;
    TIMER2_BITMODE = TIMER_BITMODE_16Bit;
    TIMER2_PRESCALER = 0;
    TIMER2_CLEAR = 1;
    TIMER2_CC[0] = 8;
    TIMER2_SHORTS = BIT(TIMER_COMPARE0_CLEAR);
    TIMER2_START = 1;
}

#define nop5()   nop(); nop(); nop(); nop(); nop()
#define nop10()  nop5(); nop5()

/* square_out -- generate square wave by bit-banging */
RAMEXEC void square_out(void) {
    while (1) {
        gpio_out(PAD0, 0);
        nop10(); nop10(); nop10(); nop();
        gpio_out(PAD0, 1);
        nop10(); nop10(); nop5(); nop(); nop(); nop(); nop();
    }
}    

void init(void) {
    gpio_dir(PAD0, 1);
    gpio_dir(PAD1, 1);
    gpio_dir(PAD2, 1);

    rng_init();
    sqwave_init();

    square_out();
}
