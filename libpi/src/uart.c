// implement:
//  void uart_init(void)
//
//  int uart_can_get8(void);
//  int uart_get8(void);
//
//  int uart_can_put8(void);
//  void uart_put8(uint8_t c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

enum {
    AUX_BASE = 0x20215000,
    AUXENB = AUX_BASE + 0x4,
    AUXIIR = AUX_BASE + 0x48,
    AUXCNTL = AUX_BASE + 0x60,
    AUXBAUD = AUX_BASE + 0x68,
    AUXIER = AUX_BASE + 0x44,
    AUXLCR = AUX_BASE + 0x4C,
    AUXIO = AUX_BASE + 0x40,
    AUXLSR = AUX_BASE + 0x54
};

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    unsigned val;
    unsigned mask;

    // turn on the UART in AUX
    dev_barrier();
    val = GET32(AUXENB);
    val |= 0x1;
    PUT32(AUXENB, val);

    // disable tx/rx
    dev_barrier();
    PUT32(AUXCNTL, 0b00);

    // clear fifo
    dev_barrier();
    PUT32(AUXIIR, 0b110);

    // disable interrupts
    dev_barrier();
    PUT32(AUXIER, 0b00);

    // set 8-bit mode
    dev_barrier();
    PUT32(AUXLCR, 0b11); // errata

    // set baud_rate
    dev_barrier();
    PUT32(AUXBAUD, 271);

    // set gpio
    dev_barrier();
    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);

    // enable tx/rx
    dev_barrier();
    PUT32(AUXCNTL, 0b11);

    uart_flush_tx();
}

// disable the uart.
void uart_disable(void) {
    dev_barrier();
    uart_flush_tx();
    unsigned val = GET32(AUXENB);
    val &= ~0x1;
    PUT32(AUXENB, val);
    dev_barrier();
}


// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_get8(void) {
    dev_barrier();
	while (!uart_has_data()) {}
    unsigned val = GET32(AUXIO);
    int ret = val & 0xFF;
    dev_barrier();
    return ret;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_put8(void) {
    dev_barrier();
    unsigned val = GET32(AUXLSR);
    int ret = (val >> 5) & 0x1;
    dev_barrier();
    return ret;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
// returns < 0 on error.
int uart_put8(uint8_t c) {
    dev_barrier();
    while (!uart_can_put8()) {}
    PUT32(AUXIO, c);
    dev_barrier();
    return 0;
}

// simple wrapper routines useful later.

// 1 = at least one byte on rx queue, 0 otherwise
int uart_has_data(void) {
    dev_barrier();
    unsigned val = GET32(AUXLSR);
    int ret = val & 0x1;
    dev_barrier();
    return ret;
}

// return -1 if no data, otherwise the byte.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    dev_barrier();
    unsigned val = GET32(AUXLSR);
    int ret = (val >> 6) & 0x1;
    dev_barrier();
    return ret;
}

// flush out all bytes in the uart --- we use this when 
// turning it off / on, etc.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
