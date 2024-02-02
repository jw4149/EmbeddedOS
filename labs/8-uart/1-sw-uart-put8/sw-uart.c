#include "rpi.h"
#include "sw-uart.h"
#include "cycle-count.h"
#include "cycle-util.h"
#include "wait-routines.h"

#include <stdarg.h>

// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_put8(sw_uart_t *uart, uint8_t c) {
    // use local variables to minimize any loads or stores
    int tx = uart->tx;
    uint32_t n = uart->cycle_per_bit,
             u = n,
             s = cycle_cnt_read();

    // todo("implement this code\n");
    // start bit (default is 1, so start = writing 0.
    gpio_write(tx,0); wait_ncycles_exact(s,u);  u +=n;

    // the byte.
    gpio_write(tx, (c>> 0)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 1)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 2)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 3)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 4)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 5)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 6)&1); wait_ncycles_exact(s, u);  u +=n;
    gpio_write(tx, (c>> 7)&1); wait_ncycles_exact(s, u);  u +=n;

    // stop bit (has to be 1 for at least n cycles)
    gpio_write(tx,1); wait_ncycles_exact(s,u); 
}

// do this second: you can type in pi-cat to send stuff.
//      EASY BUG: if you are reading input, but you do not get here in 
//      time it will disappear.
int sw_uart_get8_timeout(sw_uart_t *uart, uint32_t timeout_usec) {
    unsigned rx = uart->rx;

    // right away (used to be return never).
    while(!wait_until_usec(rx, 0, timeout_usec))
        return -1;

    uint32_t n = uart->cycle_per_bit,
             u = n,
             s = cycle_cnt_read();

    gpio_read(rx); wait_ncycles_exact(s, u); u += n;

    int bit0 = gpio_read(rx) << 0; wait_ncycles_exact(s, u); u +=n;
    int bit1 = gpio_read(rx) << 1; wait_ncycles_exact(s, u); u +=n;
    int bit2 = gpio_read(rx) << 2; wait_ncycles_exact(s, u); u +=n;
    int bit3 = gpio_read(rx) << 3; wait_ncycles_exact(s, u); u +=n;
    int bit4 = gpio_read(rx) << 4; wait_ncycles_exact(s, u); u +=n;
    int bit5 = gpio_read(rx) << 5; wait_ncycles_exact(s, u); u +=n;
    int bit6 = gpio_read(rx) << 6; wait_ncycles_exact(s, u); u +=n;
    int bit7 = gpio_read(rx) << 7; wait_ncycles_exact(s, u); u +=n;

    return bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0;
}

// finish implementing this routine.  
sw_uart_t sw_uart_init_helper(unsigned tx, unsigned rx,
        unsigned baud, 
        unsigned cyc_per_bit,
        unsigned usec_per_bit) 
{
    assert(tx && tx<31);
    assert(rx && rx<31);
    assert(cyc_per_bit && cyc_per_bit > usec_per_bit);
    assert(usec_per_bit);

    // make sure it makes sense.
    unsigned mhz = 700 * 1000 * 1000;
    unsigned derived = cyc_per_bit * baud;
    if(! ((mhz - baud) <= derived && derived <= (mhz + baud)) )
        panic("too much diff: cyc_per_bit = %d * baud = %d\n", 
            cyc_per_bit, cyc_per_bit * baud);

    // todo("setup rx,tx and initial state of tx pin.");
    gpio_set_output(tx);
    gpio_set_on(tx);

    return (sw_uart_t) { 
            .tx = tx, 
            .rx = rx, 
            .baud = baud, 
            .cycle_per_bit = cyc_per_bit ,
            .usec_per_bit = usec_per_bit 
    };
}
