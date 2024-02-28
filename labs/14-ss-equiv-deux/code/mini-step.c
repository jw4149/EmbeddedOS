// very simple code to just run a single function at user level in mismatch 
// mode.  
#include "rpi.h"
#include "armv6-debug-impl.h"
#include "mini-step.h"

// currently only handle a single breakpoint.
static step_handler_t step_handler = 0;
static void *step_handler_data = 0;

// special function.  never runs, but we know if the traced code
// calls it, that they are done.
void ss_on_exit(int exitcode);

// error checking.
static int single_step_on_p;

// registers where we started single-stepping
static regs_t start_regs;

// 0. get the previous pc that we were mismatching on.
// 1. set bvr0/bcr0 for mismatch on <pc>
// 2. prefetch_flush();
// 3. return old pc.
uint32_t mismatch_pc_set(uint32_t pc) {
    assert(single_step_on_p);
    uint32_t old_pc = cp14_bvr0_get();

    // set a mismatch (vs match) using bvr0 and bcr0 on
    // <pc>
    // todo("setup mismatch on <pc> using bvr0/bcr0");
    // cp14_bcr0_disable();
    // p 13-17
    uint32_t b = cp14_bcr0_get();
    uint32_t mask = 0b111 << 20;
    b &= ~mask;
    mask = 0b11 << 14;
    b &= ~mask;
    mask = 0b1111 << 5;
    b &= ~mask;
    b |= (0b100 << 20); 
    b |= (0b1111 << 5);
    b |= 0b111;

    if(!b)
        panic("must set b to the right bits\n");

    cp14_bcr0_set(b);
    cp14_bvr0_set((uint32_t)pc);
    assert(cp14_bcr0_is_enabled());

    prefetch_flush();

    assert( cp14_bvr0_get() == pc);
    return old_pc;
}

// on/start maybe do different things?  
void mismatch_on(void) {
    assert(!single_step_on_p);
    single_step_on_p = 1;

    // can keep doing.
    cp14_enable();

    // we assume they won't jump to 0.
    mismatch_pc_set(0);
}

// disable mis-matching by just turning it off in bcr0
void mismatch_off(void) {
    assert(single_step_on_p);
    single_step_on_p = 0;

    // RMW bcr0 to disable breakpoint, 
    // make sure you do a prefetch_flush!
    // todo("turn mismatch off, but don't modify anything else");
    cp14_bcr0_disable();
    prefetch_flush();
}

// once the traced code calls this, it's done.
void ss_on_exit(int exitcode) {
    panic("should never reach this!\n");
}

// when we get a mismatch fault.
// 1. check if its for <ss_on_exit>: if so
//    switch back to start_regs.
// 2. otherwise setup the fault and call the
//    handler.  will look like 2-brkpt-test.c
//    somewhat.
// 3. when returns, set the mismatch on the 
//    current pc.
// 4. wait until the UART can get a putc before
//    return (if you don't do this what happens?)
// 5. use switch() to to resume.
static void mismatch_fault(regs_t *r) {
    uint32_t pc = r->regs[15];

    // example of using intrinsic built-in routines
    if(pc == (uint32_t)ss_on_exit) {
        output("done pc=%x: resuming initial caller\n", pc);
        switchto(&start_regs);
        not_reached();
    }

    step_fault_t f = {};
    // todo("setup fault handler and call step_handler");
    // unsigned bvr = cp14_bvr0_get();
    // pc = bvr + 4;
    f = step_fault_mk(pc, r);
    step_handler(step_handler_data, &f);

    // todo("setup a mismatch on pc");
    mismatch_pc_set(pc);

    // otherwise there is a race condition if we are 
    // stepping through the uart code --- note: we could
    // just check the pc and the address range of
    // uart.o
    while(!uart_can_put8())
        ;

    switchto(r);
}

// will look like mini_watch_init> but for
// breakpoints (not watchpoints) with prefetch
// exception.
void mini_step_init(step_handler_t h, void *data) {
    cp14_enable();
    assert(h);
    step_handler_data = data;
    step_handler = h;

    // todo("setup the rest");
    full_except_install(0);
    full_except_set_prefetch(mismatch_fault);

    // just started, should not be enabled.
    assert(!cp14_bcr0_is_enabled());
    assert(!cp14_bcr0_is_enabled());
}

// run <fn> with argument <arg> in single step mode.
uint32_t mini_step_run(void (*fn)(void*), void *arg) {
    uint32_t pc = (uint32_t)fn;

    // we use the same stack at the same address so the 
    // values check out from run to run.
    static void *stack = 0;
    enum { stack_size = 8192*4};
    
    if(!stack)
        stack = kmalloc(stack_size);

    void *sp = stack + stack_size;

    // take the current cpsr and switch to user mode and clear
    // carry flags.
    uint32_t cpsr = cpsr_inherit(USER_MODE, cpsr_get());
    assert(mode_get(cpsr) == USER_MODE);

    // setup our initial registers.  everything not set will be 0.
    regs_t r = (regs_t) {
        .regs[REGS_PC] = (uint32_t)fn,      // where we want to jump to
        .regs[REGS_SP] = (uint32_t)sp,      // the stack pointer to use.
        .regs[REGS_LR] = (uint32_t)ss_on_exit, // where to jump if return.
        .regs[REGS_CPSR] = cpsr             // the cpsr to use.
    };

    // note: we won't fault b/c we are not at user level yet!
    mismatch_on();

    // switch to <r> save values in <start_regs>
    switchto_cswitch(&start_regs, &r);

    // mistmatch is off.
    mismatch_off();

    // return what the user set.
    return r.regs[0];
}

void mismatch_run(regs_t *r) {
    uint32_t pc = r->regs[REGS_PC];

    mismatch_pc_set(pc);

    // otherwise there is a race condition if we are 
    // stepping through the uart code --- note: we could
    // just check the pc and the address range of
    // uart.o
    while(!uart_can_put8())
        ;

    switchto(r);
}
