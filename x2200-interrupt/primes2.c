// x2200-interrupt/primes2.c
// Copyright (c) 2018 J. M. Spivey

#include "hardware.h"
#include "lib.h"
#include <stdarg.h>

/* Pins to use for serial communication */
#define TX USB_TX
#define RX USB_RX

#define NBUF 64

static volatile int txidle;       // Whether UART is idle
static volatile int bufcnt = 0;   // Number of chars in buffer
static unsigned bufin = 0;        // Index of first free slot
static unsigned bufout = 0;       // Index of first occupied slot
static volatile char txbuf[NBUF]; // The buffer

/* buf_put -- add character to buffer */
void buf_put(char ch) {
    assert(bufcnt < NBUF);
    txbuf[bufin] = ch;
    bufcnt++;
    bufin = (bufin+1) % NBUF;
}

/* buf_get -- fetch character from buffer */
char buf_get(void) {
    assert(bufcnt > 0);
    char ch = txbuf[bufout];
    bufcnt--;
    bufout = (bufout+1) % NBUF;
    return ch;
}

/* serial_init -- set up UART connection to host */
void serial_init(void) {
    UART_ENABLE = UART_ENABLE_Disabled;
    UART_BAUDRATE = UART_BAUDRATE_9600; // 9600 baud
    UART_CONFIG = FIELD(UART_CONFIG_PARITY, UART_PARITY_None);
                                        // format 8N1
    UART_PSELTXD = TX;                  // choose pins
    UART_PSELRXD = RX;
    UART_ENABLE = UART_ENABLE_Enabled;
    UART_TXDRDY = 0;
    UART_STARTTX = 1;

    // Interrupt for transmit only
    UART_INTENSET = BIT(UART_INT_TXDRDY);
    enable_irq(UART_IRQ);
    txidle = 1;
}

/* uart_handler -- interrupt handler for UART */
void uart_handler(void) {
    if (UART_TXDRDY) {
        UART_TXDRDY = 0;
        if (bufcnt == 0)
            txidle = 1;
        else
            UART_TXD = buf_get();
    }
}

/* serial_putc -- send output character */
void serial_putc(char ch) {
    while (bufcnt == NBUF) pause();

    intr_disable();
    if (txidle) {
        UART_TXD = ch;
        txidle = 0;
    } else {
        buf_put(ch);
    }
    intr_enable();
}

/* print_buf -- output routine for use by printf */
void print_buf(char *buf, int n) {
    for (int i = 0; i < n; i++) {
        char c = buf[i];
        if (c == '\n') serial_putc('\r');
        serial_putc(c);
    }
}

int modulo(int a, int b) {
    int r = a;
    while (r >= b) r -= b;
    return r;
}

int prime(int n) {
    for (int k = 2; k * k <= n; k++) {
        if (modulo(n, k) == 0)
            return 0;
    }

    return 1;
}

void start_timer(void) {
    // Light an LED
    led_dot();

    // Start a timer
    TIMER0_MODE = TIMER_MODE_Timer;
    TIMER0_BITMODE = TIMER_BITMODE_32Bit;
    TIMER0_PRESCALER = 4; // Count at 1MHz
    TIMER0_START = 1;
}

void stop_timer(void) {
    // Turn LED off
    led_off();

    // Fetch timer result
    TIMER0_CAPTURE[0] = 1;
    unsigned time1 = TIMER0_CC[0];
    while (bufcnt > 0) pause();
    TIMER0_CAPTURE[0] = 1;
    unsigned time2 = TIMER0_CC[0];
    printf("%d+%d millisec\n",
           (time1+500)/1000, (time2-time1+500)/1000);
}

void init(void) {
    int n = 2, count = 0;

    led_init();
    serial_init();
    delay_loop(10000);
    start_timer();

    while (count < 500) {
        if (prime(n)) {
            count++;
            printf("prime(%d) = %d\n", count, n);
        }
        n++;
    }
    
    stop_timer();
}
