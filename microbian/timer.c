// timer.c
// Copyright (c) 2018 J. M. Spivey

#include "microbian.h"
#include "hardware.h"

#ifdef UBIT_V2
#define TICK 1                  // Interval between updates (ms)
#endif

#ifndef TICK
#define TICK 5                  // Sensible default
#endif

#define MAX_TIMERS 8

/* Millis will overflow in about 46 days, but that's long enough. */
static unsigned millis = 0;

static struct {
    int client;        // Process that receives message, or -1 if free
    unsigned period;   // Interval between messages, or 0 for one-shot
    unsigned next;     // Next time to send a message
} timer[MAX_TIMERS];

/* check_timers -- send any messages that are due */
static void check_timers(void) {
    int i;
    message m;

    for (i = 0; i < MAX_TIMERS; i++) {
        if (timer[i].client >= 0 && millis >= timer[i].next) {
            m.m_i1 = timer[i].next;
            send(timer[i].client, PING, &m);

            if (timer[i].period > 0)
                timer[i].next += timer[i].period;
            else
                timer[i].client = -1;

        }
    }
}

/* create -- create a new timer */
static void create(int client, int delay, int repeat) {
    int i = 0;

    while (i < MAX_TIMERS && timer[i].client >= 0)
        i++;

    if (i == MAX_TIMERS)
        panic("Too many timers");

    timer[i].client = client;
    if (repeat) {
        timer[i].next = millis + delay;
        timer[i].period = delay;
    } else {
        // Add TICK to be sure that the timer does not go off early
        timer[i].next = millis + delay + TICK;
        timer[i].period = 0;
    }
}

static int TIMER_TASK;

/* timer1_handler -- interrupt handler */
void timer1_handler(void) {
    // Update the time here so it is accessible to timer_micros

    if (TIMER1_COMPARE[0]) {
        millis += TICK;
        TIMER1_COMPARE[0] = 0;
        interrupt(TIMER_TASK);
    }
}

static void timer_task(int n) {
    message m;

    /* We use Timer 1 because its 16-bit mode is adequate for a clock
       with up to 1us resolution and 5ms period, leaving the 32-bit
       Timer 0 for other purposes. */

    TIMER1_STOP = 1;
    TIMER1_MODE = TIMER_MODE_Timer;
    TIMER1_BITMODE = TIMER_BITMODE_16Bit;
    TIMER1_PRESCALER = 4;      // 1MHz = 16MHz / 2^4
    TIMER1_CLEAR = 1;
    TIMER1_CC[0] = 1000 * TICK;
    TIMER1_SHORTS = BIT(TIMER_COMPARE0_CLEAR);
    TIMER1_INTENSET = BIT(TIMER_INT_COMPARE0);
    TIMER1_START = 1;
    enable_irq(TIMER1_IRQ);

    while (1) {
        receive(ANY, &m);

        switch (m.m_type) {
        case INTERRUPT:
            check_timers();
            break;

        case REGISTER:
            create(m.m_sender, m.m_i1, m.m_i2);
            break;

        default:
            badmesg(m.m_type);
        }
    }
}

void timer_init(void) {
    int i;

    for (i = 0; i < MAX_TIMERS; i++)
        timer[i].client = -1;

    TIMER_TASK = start("Timer", timer_task, 0, 256);
}

/* timer_now -- return current time in milliseconds since startup */
unsigned timer_now(void) {
    return millis;
}

/* timer_micros -- return microseconds since startup */
unsigned timer_micros(void) {
    unsigned my_millis, ticks1, ticks2, extra;
    
    /* We must allow for the possibility the timer has overflowed but
       the interrupt has not yet been handled. Worse, the timer
       overflow could happen between looking at the timer and looking
       at the interrupt flag. */

    intr_disable();
    TIMER1_CAPTURE[1] = 1;      // Capture count before testing irq
    extra = TIMER1_COMPARE[0];  // Inspect the IRQ
    TIMER1_CAPTURE[2] = 1;      // Capture count afterwards
    ticks1 = TIMER1_CC[1];
    ticks2 = TIMER1_CC[2];
    my_millis = millis;
    intr_enable();

    /* No correction if overflow happened after the first observation */
    if (extra && ticks1 <= ticks2)
        my_millis += TICK;

    return 1000 * my_millis + ticks1;
}

/* timer_delay -- one-shot delay */
void timer_delay(int msec) {
    message m;
    m.m_i1 = msec;
    m.m_i2 = 0;                 /* Don't repeat */
    send(TIMER_TASK, REGISTER, &m);
    receive(PING, NULL);
}

/* timer_pulse -- regular pulse */
void timer_pulse(int msec) {
    message m;
    m.m_i1 = msec;
    m.m_i2 = 1;                /* Repetitive */
    send(TIMER_TASK, REGISTER, &m);
}

/* wait -- sleep until next timer pulse */
void timer_wait(void) {
    receive(PING, NULL);
}

