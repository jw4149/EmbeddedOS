#include "rpi.h"
#include "mbox.h"

uint32_t rpi_temp_get(void) ;

#include "cycle-count.h"

// compute cycles per second using
//  - cycle_cnt_read();
//  - timer_get_usec();
unsigned cyc_per_sec(void) {
    return 1;
}


void notmain(void) { 
    output("mailbox serial number = %llx\n", rpi_get_serialnum());

    output("mailbox revision number = %x\n", rpi_get_revision());
    output("mailbox model number = %x\n", rpi_get_model());

    uint32_t size = rpi_get_memsize();
    output("mailbox physical mem: size=%d (%dMB)\n", 
            size, 
            size/(1024*1024));

    // print as fahrenheit
    unsigned x = rpi_temp_get();

    // convert <x> to C and F
    unsigned C = x/1000;
    unsigned F = (9*C)/5+32;
    output("mailbox temp = %x, C=%d F=%d\n", x, C, F); 

    // todo("do overclocking!\n");
}
