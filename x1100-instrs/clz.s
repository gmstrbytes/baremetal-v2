@ x1100/func.s
@ Copyright (c) 2018-20 J. M. Spivey

        .syntax unified
        .global func

        .text
        .thumb_func
func:
@ ----------------
@ Two parameters are in registers r0 and r1

        clz r0, r0              @ Count leading zeros

@ Result is now in register r0
@ ----------------
        bx lr                   @ Return to the caller

