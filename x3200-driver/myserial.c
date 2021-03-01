// x3200-driver/myserial.c
// Copyright (c) 2020 J. M. Spivey

#include "microbian.h"
#include "hardware.h"
#include <stdarg.h>

#define TX USB_TX
#define RX USB_RX

static int SERIAL_TASK;

/* Message types for serial task */
#define PUTC 16

/* NBUF -- size of output buffer.  Should be a power of 2. */
#define NBUF 128

/* Output buffer */
static char txbuf[NBUF];        /* Circular buffer for output */
static int bufin = 0;          /* In pointer */
static int bufout = 0;         /* Out pointer */
static int n_tx = 0;            /* Character count */

static int txidle = 1;          /* True if transmitter is idle */

/* The clear_pending() call below is needed because the UART interrupt
   handler disables the IRQ for the UART in the NVIC, but doesn't
   disable the UART itself from sending interrupts.  The pending bit
   is cleared on return from the interrupt handler, but that doesn't
   stop the UART from setting it again. */

/* serial_interrupt -- handle serial interrupt */
static void serial_interrupt(void) {
    if (UART_TXDRDY) {
        txidle = 1;
        UART_TXDRDY = 0;
    }

    clear_pending(UART_IRQ);
    enable_irq(UART_IRQ);
}

/* start_tx -- start transmitter if possible */
static void start_tx(void) {
    // Can we start transmitting a character?
    if (txidle && n_tx > 0) {
        UART_TXD = txbuf[bufout];
        bufout = (bufout+1) % NBUF;
        n_tx--;
        txidle = 0;
    }
}

/* serial_task -- driver process for UART */
static void serial_task(int arg) {
    message m;
    char ch;

    UART_ENABLE = UART_ENABLE_Disabled;
    UART_BAUDRATE = UART_BAUDRATE_9600; // 9600 baud
    UART_CONFIG = FIELD(UART_CONFIG_PARITY, UART_PARITY_None);
                                        // format 8N1
    UART_PSELTXD = TX;                  // choose pins
    UART_PSELRXD = RX;
    UART_ENABLE = UART_ENABLE_Enabled;
    UART_STARTTX = 1;

    UART_INTENSET = BIT(UART_INT_TXDRDY);
    connect(UART_IRQ);
    enable_irq(UART_IRQ);
    txidle = 1;

    while (1) {
        receive(ANY, &m);

        switch (m.m_type) {
        case INTERRUPT:
            serial_interrupt();
            break;

        case PUTC:
            ch = m.m_i1;
            
            while (n_tx == NBUF) {
                // The buffer is full -- wait for a space to appear
                receive(INTERRUPT, NULL);
                serial_interrupt();
                start_tx();
            }

            txbuf[bufin] = ch;
            bufin = (bufin+1) % NBUF;
            n_tx++;
            break;

        default:
            badmesg(m.m_type);
        }
          
        start_tx();
    }
}

/* myserial_init -- start the serial driver task */
void myserial_init(void) {
    SERIAL_TASK = start("Serial", serial_task, 0, 256);
}

/* myserial_putc -- queue a character for output */
void myserial_putc(char ch) {
    message m;
    m.m_i1 = ch;
    send(SERIAL_TASK, PUTC, &m);
}

/* print_buf -- output routine for use by printf */
void print_buf(char *buf, int n) {
    for (int i = 0; i < n; i++) {
        char c = buf[i];
        if (c == '\n') myserial_putc('\r');
        myserial_putc(c);
    }
}
