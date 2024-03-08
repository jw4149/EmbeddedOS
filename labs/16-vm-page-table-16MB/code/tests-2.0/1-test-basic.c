// complete working example of trivial vm:
//  1. identity map the kernel (code, heap, kernel stack, 
//     exception stack, device memory)
//  2. turn VM on/off making sure we can print.
//  3. with VM on, write to an unmapped address and make sure we 
//     get a fault.
#include "rpi.h"
#include "pt-vm.h"

#if 0
#include "pinned-vm.h"
#include "mmu.h"       

// from last lab.
#include "switchto.h"
#include "full-except.h"
#endif

enum { SixteenMB = 1024*1024*16};

// used to store the illegal address we will write.
static volatile uint32_t illegal_addr;

// a trivial fault handler that checks that we got the fault
// we expected.
static void fault_handler(regs_t *r) {
    uint32_t fault_addr;

    // b4-44
    asm volatile("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_addr));

    // make sure we faulted on the address that should be accessed.
    if(fault_addr != illegal_addr)
        panic("illegal fault!  expected %x, got %x\n",
            illegal_addr, fault_addr);
    else
        trace("SUCCESS!: got a fault on address=%x\n", fault_addr);

    // done with test.
    trace("all done: going to reboot\n");
    clean_reboot();
}

void notmain(void) { 
    assert(!mmu_is_enabled());

    // ******************************************************
    // 1. one-time initialization.
    //   - create an empty page table (to catch errors).
    //   - initialize all the vm hardware
    //   - compute permissions for no-user-access.


    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)SixteenMB, SixteenMB);

    vm_pt_t *pt = vm_pt_alloc(PT_LEVEL1_N);

    // initialize everything, after bootup.
    // <mmu.h>
    vm_mmu_init(dom_no_access);

    // definitions in <pinned-vm.h>
    //
    // see 3-151 for table, or B4-9
    //    given: APX = 0, AP = 1
    //    the bits are: APX << 2 | AP
    uint32_t AXP = 0;
    uint32_t AP = 1;
    uint32_t no_user = AXP << 2 | 1; // no access user (privileged only)
    assert(perm_rw_priv == no_user);

    // armv6 has 16 different domains with their own privileges.
    // just pick one for the kernel.
    enum { 
        dom = 0, // domain id for kernel
    };          

    // ******************************************************
    // 2. setup device memory.
    // 
    // permissions: kernel domain, no user access, 
    // memory rules: strongly ordered, not shared.
    pin_t dev  = pin_mk_global(dom, no_user, MEM_device);
    
    // map all device memory to itself.  ("identity map")
    vm_map_sec(pt, 0x20000000, 0x20000000, dev);
    vm_map_sec(pt, 0x21000000, 0x21000000, dev);
    vm_map_sec(pt, 0x22000000, 0x22000000, dev);

    // ******************************************************
    // 3. setup kernel memory: 

    // protection: same as device.
    // memory rules: uncached access.
    pin_t kern = pin_mk_global(dom, no_user, MEM_uncached);

    // Q1: if you uncomment this, what happens / why?
    // kern = pin_mk_global(dom_kern, perm_ro_priv, MEM_uncached);

    // map first two MB for the kernel (1: code, 2: heap).
    //
    // NOTE: obviously it would be better to not have 0 (null) 
    // mapped, but our code starts at 0x8000 and we are using
    // 1mb sections.  you can fix this as an extension.  
    // very useful!
    vm_map_sec(pt, 0, 0, kern);              
    vm_map_sec(pt, SixteenMB, SixteenMB, kern);     

    // now map kernel stack (or nothing will work)
    uint32_t kern_stack = STACK_ADDR-SixteenMB;
    vm_map_sec(pt, kern_stack, kern_stack, kern);   // tlb 5
    uint32_t except_stack = INT_STACK_ADDR-SixteenMB;

    // Q2: if you comment this out, what happens?
    vm_map_sec(pt, except_stack, except_stack, kern);

    // ******************************************************
    // 4. setup vm hardware.
    //  - page table, asid, pid.
    //  - domain permissions.

    // b4-42: give permissions for all domains.

    // Q3: if you set this to ~0, what happens w.r.t. Q1?
    domain_access_ctrl_set(DOM_client << dom*2); 

    // set address space id, page table, and pid.
    // note:
    //  - pid never matters, it's just to help the os.
    //  - asid doesn't matter for this test b/c all entries 
    //    are global
    //  - the page table is empty (since pinning) and is
    //    just to catch errors.
    enum { ASID = 1, PID = 128 };
    mmu_set_ctx(PID, ASID, pt);

    // if you want to see the lockdown entries.
    // lockdown_print_entries("about to turn on first time");

    // ******************************************************
    // 5. turn it on/off, checking that it worked.
    trace("about to enable\n");
    for(int i = 0; i < 10; i++) {
        mmu_enable();

        if(mmu_is_enabled())
            trace("MMU ON: hello from virtual memory!  cnt=%d\n", i);
        else
            panic("MMU is not on?\n");

        mmu_disable();
        assert(!mmu_is_enabled());
        trace("MMU is off!\n");
    }

    // ******************************************************
    // 6. setup exception handling and make sure we get a fault.

    // just like last lab.  setup a data abort handler.
    full_except_install(0);
    full_except_set_data_abort(fault_handler);

    // the address we will write to (2MB) we know this is not mapped.
    illegal_addr = SixteenMB + SixteenMB;

    // this <PUT32> should "work" since vm is off.
    assert(!mmu_is_enabled());
    PUT32(illegal_addr, 0xdeadbeef);
    trace("we wrote without vm: got %x\n", GET32(illegal_addr));
    assert(GET32(illegal_addr) == 0xdeadbeef);

    // this should fault.
    vm_mmu_enable();
    assert(mmu_is_enabled());
    GET32(8 * 1024 * 1024);
    printk("got here");
    PUT32(illegal_addr, 0xdeadbeef);

    panic("should not reach here\n");
}
