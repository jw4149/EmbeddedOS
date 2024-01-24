// do a system call from user level.
//   can check cpsr to see that the 
// you have to write:
//  - syscall-asm.S:run_user_code_asm to switch to USER mode.
//  - syscall_vector:(below) to check that it came from USER level.
//  - notmain:(below) to setup stack and install handlers.
//  - _int_table_user[], _int_table_user_end[] tables in interrupt-asm.S
#include "rpi.h"
#include "rpi-interrupts.h"


// in syscall-asm.S
void run_user_code_asm(void (*fn)(void), void *stack);

// run <fn> at user level: <stack> must be 8 byte
// aligned
void run_user_code(void (*fn)(void), void *stack) {
    assert(stack);
    demand((unsigned)stack % 8 == 0, stack must be 8 byte aligned);

    run_user_code_asm(fn, stack);
    not_reached();
}

void interrupt_vector(unsigned pc) {
    panic("should not be called\n");
}

// example of using inline assembly to get the cpsr
// you can also use assembly routines.
static inline uint32_t cpsr_get(void) {
    uint32_t cpsr;
    asm volatile("mrs %0,cpsr" : "=r"(cpsr));
    return cpsr;
}

enum { N = 1024 * 64 };
static uint64_t stack[N];

/*********************************************************************
 * modify below.
 */

// this should be running at user level: you don't have to change this
// routine.
void user_fn(void) {
    uint64_t var;

    trace("checking that stack got switched\n");
    assert(&var >= &stack[0]);
    assert(&var < &stack[N]);


    // you should put the <cpsr> mode in <mode>
    unsigned mode = 0;
    todo("check that the current mode is USER_LEVEL");


    if(mode != USER_MODE)
        panic("mode = %b: expected %b\n", mode, USER_MODE);
    else
        trace("cpsr is at user level\n");

    trace("about to call hello\n");
    asm volatile("swi 1" ::: "memory");

    trace("about to call exit\n");
    asm volatile("swi 2" ::: "memory");

    not_reached();
}


// pc should point to the system call instruction.
//      can see the encoding on a3-29:  lower 24 bits hold the encoding.
int syscall_vector(unsigned pc, uint32_t r0) {
    uint32_t inst, sys_num, mode;

    todo("get <spsr> and check that mode bits = USER level\n");


    // do not change this code!
    if(mode != USER_MODE)
        panic("mode = %b: expected %b\n", mode, USER_MODE);
    else
        trace("success: spsr is at user level: mode=%b\n", mode);

    // we do a very trivial hello and exit just to show the difference
    switch(sys_num) {
    case 1: 
            trace("syscall: hello world\n");
            return 0;
    case 2: 
            trace("exiting!\n");
            clean_reboot();
    default: 
            printk("illegal system call = %d!\n", sys_num);
            return -1;
    }
}

void notmain() {
    todo("use int_init_vec to install vector with a different swi handler");

#if 0
    // you'll have to define these two symbols: swi instructions
    // should get routed to <syscall_vector>
    extern uint32_t _int_table_user[], _int_table_user_end[];
    int_init_vec(_int_table_user, _int_table_user_end);
#endif
    todo("define int_table_user interrupts-asm.S: should set stack pointer!");

    todo("set <sp> to a reasonable stack address in <stack>");
    uint64_t *sp = 0;

    output("calling user_fn with stack=%p\n", sp);
    // will call user_fn with stack pointer <sp>
    run_user_code(user_fn, sp); 
    not_reached();
}
