/* x3700/car.c */
/* Copyright (c) 2021 J. M. Spivey */

#include "microbian.h"
#include "hardware.h"


int ADAPTER, CONTROL;

/* RADIO ADAPTER PROCESS */

/* Message type for remote command */
#define CMD 24

void adapter_task(int dummy)
{
    message m;
    int n;
    byte buf[32];

    while (1) {
        n = radio_receive(buf);
        if (n >= 2 && buf[0] == '@') {
            m.m_i1 = buf[1];
            send(CONTROL, CMD, &m);
        }
    }
}


/* CONTROL PROCESS */

void pwm_change(int width0, int width1);
void pwm_init(void);

#define REST 1500
#define SPEED 300
#define TURN 150

static int quantum = 0;

int lspeed = 0, rspeed = 0;

void drive(int lsp, int rsp, int t)
{
    if (lsp != lspeed || rsp != rspeed) {
        lspeed = lsp; rspeed = rsp;
        pwm_change(REST-lspeed, REST+rspeed);
    }
    quantum = t;
}

void stop(void)
{
    /* We disconnect the motors to avoid the problem of creep */
    lspeed = rspeed = 0;
    pwm_change(0, 0);
}

void control_task(int dummy)
{
    message m;
    char cmd;

    stop();
    timer_pulse(20);

    while (1) {
        receive(ANY, &m);
        switch (m.m_type) {
        case PING:
            if (quantum > 0) {
                quantum--;
                if (quantum == 0) stop();
            }
            break;

        case CMD:
            cmd = m.m_i1;
            if (cmd == 'B')
                drive(SPEED, SPEED, 25);
            else if (cmd == 'L')
                drive(TURN, -TURN, 10);
            else if (cmd == 'R')
                drive(-TURN, TURN, 10);
            break;

        default:
            panic("Controller got bad command");
        }
    }
}

void init(void)
{
    timer_init();
    radio_init();
    pwm_init();
    ADAPTER = start("Adapter", adapter_task, 0, STACK);
    CONTROL = start("Control", control_task, 0, STACK);
}
