@ Must carefully read B2 of the armv6 manual.  You cannot rely on "well it
@ worked on my test case": mistakes cause stale data, which may or may not
@ show up in your test case (likely not) despite being wildly broken.
@
@ Note: Rd is allegedly a read-only parameter to these instructions, but
@ the manual says SBZ (should be zero).  I think (and the linux source seems
@ to think) that this means we have to explicitly set it to 0.  Not setting
@ Rd=0 is an easy mistake to make.  Perhaps is worth building in (Linux does
@ not, but) --- the cost of an extra mov is negligible given our other 
@ overheads.
@
@ Alot of people put these instructions in inline assembly, wrapped up by 
@ function calls (e.g., cs107e's MMU code).  This practice is dangerous. 
@ For example, if you are flushing caches, you cannot be sure that the 
@ function return, etc does not then reload them, esp w.r.t. the BTB.  I 
@ think the only safe approach is to put directly in assembly so you are 
@ guaranteed no branch, indirect jump, load/store happens in between.
@
@ A lot of MMU code out there appears pretty broken b/c they don't do enough
@ flushing operations and/or rely on gcc code generation to not do the above.

#include "rpi-asm.h"
#include "armv6-coprocessor-asm.h"


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ many page numbers are in 
@    armv6-coprocessor-asm.h
@ and the other armv6-*.h files

// just read the cache type: use C code to figure out if it is unified
// b6-13, b6-14
MK_FN(get_cache_type)
    asm_not_implemented()
    bx lr

// void reset(void)
// clear the dcache, icache, tlbs
MK_FN(mmu_reset)
    @ b staff_mmu_reset
    mov r2, #0
    INV_TLB(r2)
    INV_DCACHE(r2)
    INV_ICACHE(r2)
    @ FLUSH_BTB(r2)
    DSB(r2)
    PREFETCH_FLUSH(r2)
    bx lr

// void cp15_domain_ctrl_wr(uint32_t dom_reg);
//  need to do cp15 sync: flush_btb, dsb, prefetch flush
MK_FN(cp15_domain_ctrl_wr)
    @ b staff_cp15_domain_ctrl_wr
    mov r2, #0
    DOMAIN_CTRL_WR(r0)
    PREFETCH_FLUSH(r2)
    bx lr

// void cp15_set_procid_ttbr0(uint32_t proc_and_asid, fld_t *pt);
// sequence from b2-25
MK_FN(cp15_set_procid_ttbr0)
    @ b staff_cp15_set_procid_ttbr0

    mov r2, #0
    ASID_SET(r2)
    PREFETCH_FLUSH(r2)

    TTBR_BASE_CTRL_WR(r2)
    TTBR0_SET(r1)
    TTBR1_SET(r2)
    PREFETCH_FLUSH(r2)
    FLUSH_BTB(r2)
    PREFETCH_FLUSH(r2)

    ASID_SET(r0)
    PREFETCH_FLUSH(r2)
    bx lr

// void mmu_disable_set_asm(cp15_ctrl_reg1_t c);
MK_FN(mmu_disable_set_asm)
    @ b staff_mmu_disable_set_asm

    @ note: this piece of code is mapped into the sample place in both
    @ MMU-enabled and MMU-disabled so BTB and Prefetch entries work identically
    @ to both MMU-enable and disable.   Without this requirement I believe is
    @ very hard to get right (perhaps impossible?) since we cannot flush
    @ the BTB and prefetch and switch MMU on/off atomically.  thus, would
    @ flush, but instructions could get fetched before we could disable,
    @ defeating the purpose.

    mov r2, #0
    CLEAN_INV_DCACHE(r2)
    DSB(r2)
    PREFETCH_FLUSH(r2)

    CONTROL_REG1_WR(r0)
    PREFETCH_FLUSH(r2)

    INV_ICACHE(r2)
    DSB(r2)
    PREFETCH_FLUSH(r2)

    FLUSH_BTB(r2)
    PREFETCH_FLUSH(r2)
    bx lr

// void mmu_enable_set_asm(cp15_ctrl_reg1_t c);
MK_FN(mmu_enable_set_asm)
    @ b staff_mmu_enable_set_asm
    mov r2, #0

    CONTROL_REG1_RD(r3)
    bic r3, r3, #0x1000
    CONTROL_REG1_WR(r3)
    PREFETCH_FLUSH(r2)

    INV_ICACHE(r2)
    DSB(r2)

    CONTROL_REG1_WR(r0)
    PREFETCH_FLUSH(r2)

    FLUSH_BTB(r2)
    PREFETCH_FLUSH(r2)
    bx lr

// B-23
MK_FN(mmu_sync_pte_mods)
    @ b staff_mmu_sync_pte_mods
    mov r2, #0
    CLEAN_INV_DCACHE(r2)
    INV_ICACHE(r2)
    DSB(r2)
    INV_TLB(r2)
    FLUSH_BTB(r2)
    DSB(r2)
    PREFETCH_FLUSH(r2)
    bx lr

