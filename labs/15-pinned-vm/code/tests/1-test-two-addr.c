#include "rpi.h"
#include "pinned-vm.h"
#include "mmu.h"

void notmain(void) { 
    enum { OneMB = 1024*1024};
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)MB, MB);
    assert(!mmu_is_enabled());

    // b4-42
    // staff_domain_access_ctrl_set(~0);   // DOM_client << DOM);
    enum { dom_kern = 1,            // domain for kernel=1
           dom_user = 2 };          // domain for user = 2
    uint32_t d = (DOM_client << dom_kern*2)
                |(DOM_client << dom_user*2);

    pin_mmu_init(d);


    // see 3-151 for table, or B4-9
    //    APX = 0, AP = 1
    //    APX << 2 | AP
    uint32_t no_user = perm_rw_priv;

    // description of device memory
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    // description of kernel memory
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    unsigned idx = 0;
    // all the device memory: identity map
    pin_mmu_sec(idx++, 0x20000000, 0x20000000, dev);
    pin_mmu_sec(idx++, 0x20100000, 0x20100000, dev);
    pin_mmu_sec(idx++, 0x20200000, 0x20200000, dev);

    // map first two MB for the kernel
    pin_mmu_sec(idx++, 0, 0, kern);
    pin_mmu_sec(idx++, OneMB, OneMB, kern);

    // kernel stack
    pin_mmu_sec(idx++, STACK_ADDR-OneMB, STACK_ADDR-OneMB, kern);

    // our interrupt stack.
    // pin_mmu_sec(idx++, INT_STACK_ADDR-OneMB, INT_STACK_ADDR-OneMB, kern);

    enum { ASID1 = 1, ASID2 = 2 };
    pin_t user1 = pin_16mb(pin_mk_user(dom_kern, ASID1, no_user, MEM_uncached));
    pin_t user2 = pin_16mb(pin_mk_user(dom_kern, ASID2, no_user, MEM_uncached));
    
    uint32_t user_addr = OneMB * 16;
    assert((user_addr>>12) % 16 == 0);
    uint32_t phys_addr1 = user_addr;
    uint32_t phys_addr2 = user_addr+16*OneMB;
    
    PUT32(phys_addr1, 0xdeadbeef);
    PUT32(phys_addr2, 0xdeadbeef);

    uint32_t user_idx = idx;
    pin_mmu_sec(idx++, user_addr, phys_addr1, user1);
    assert(idx<8);

    trace("about to enable\n");


    lockdown_print_entries("about to turn on first time");

    pin_mmu_switch(0,ASID1);
    pin_mmu_enable();

    assert(mmu_is_enabled());
    trace("MMU is on and working!\n");
    
    uint32_t x = GET32(user_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xdeadbeef);
    PUT32(user_addr, 1);

    pin_mmu_disable();
    assert(!mmu_is_enabled());
    trace("MMU is off!\n");
    trace("phys addr1=%x\n", GET32(phys_addr1));
    assert(GET32(phys_addr1) == 1);

    pin_mmu_sec(user_idx, user_addr, phys_addr2, user2);

    lockdown_print_entries("about to turn on");

    pin_mmu_switch(0,ASID2);
    staff_mmu_enable();

    x = GET32(user_addr);
    trace("asid 2 = got: %x\n", x);
    assert(x == 0xdeadbeef);
    PUT32(user_addr, 2);

    pin_mmu_disable();
    trace("checking user2=%x\n", GET32(phys_addr2));
    assert(GET32(phys_addr2) == 2);

    trace("SUCCESS!\n");
}
