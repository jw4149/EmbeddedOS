#include "rpi-asm.h"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ <0-rfe-example.c>

@ use rfe to load pc and cpsr from the 2 entry
@ array passed in r0.
MK_FN(rfe_asm)
    mov sp, r0
    rfe sp
    asm_not_reached();

@ a trampoline that loads the stack pointer
@ and calls <rfe_c_code>
MK_FN(rfe_trampoline_asm)
    mov sp, #INT_STACK_ADDR
    mov r0, sp
    bl rfe_c_code
    asm_not_reached();

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ <1-rfe-blk.c>

@ tiny change: modify rfe_asm to deal with being 
@ passed a 17-entry register array
MK_FN(blk_rfe_asm)
    mov r1, #15
    lsl r1, #2
    add sp, r0, r1
    rfe sp
    asm_not_reached();

@ do not need to modify: same as original, so just
@ call it.
MK_FN(blk_trampoline_asm)
    bl rfe_trampoline_asm
    asm_not_reached();


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ <1-rfe-load-sp.c>

@ copy-paste <blk_rfe_asm> above and change it 
@ so it:
@  1. loads r0-r14 into user mode registers
@     (hint: look back at your <0-user-sp-lr> 
@     code)
@  2. does an rfe 
MK_FN(switchto_mode_asm)
    mov r1, #15
    lsl r1, #2
    add sp, r0, r1

    mov r1, #13
    lsl r1, #2
    add r1, r0, r1
    mov r2, #14
    lsl r2, #2
    add r2, r0, r2
    mov r3, #16
    lsl r3, #2
    add r3, r0, r3
    ldr r3, [r3]

    mrs r12, cpsr
    msr cpsr_c, r3
    prefetch_flush(r13)
    ldm r1, {sp}
    ldm r2, {lr}
    msr cpsr_c, r12
    prefetch_flush(r13)

    ldm r0, {r0-r12}
    rfe sp
    asm_not_reached();

@ so we don't get linker errors 
.weak rfe_c_code
rfe_c_code:
    asm_not_reached()
