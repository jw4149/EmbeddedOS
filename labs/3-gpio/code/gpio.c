/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    if(pin >= 32)
        return;

  // implement this
  // use <gpio_fsel0>
  unsigned reg_n = pin / 10;
  unsigned sel_n = pin % 10;

  unsigned *base_pointer = (void *)GPIO_BASE;
  unsigned addr = (unsigned)(base_pointer + reg_n);

  unsigned val = GET32(addr);

  unsigned mask = 0x7 << (sel_n * 3);
  unsigned shift = 3 * sel_n;

  val &= ~mask;
  val |= (1 << shift);
  PUT32(addr, val);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32)
        return;
  // implement this
  // use <gpio_set0>
  PUT32(gpio_set0, (1 << pin));
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32)
        return;
  // implement this
  // use <gpio_clr0>
  PUT32(gpio_clr0, (1 << pin));
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  // implement.
  unsigned reg_n = pin / 10;
  unsigned sel_n = pin % 10;

  unsigned *base_pointer = (void *)GPIO_BASE;
  unsigned addr = (unsigned)(base_pointer + reg_n);

  unsigned val = GET32(addr);

  unsigned mask = 0x7 << (sel_n * 3);
  val &= ~mask;

  PUT32(addr, val);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  unsigned v = 0;

  // implement.
  unsigned val = GET32(gpio_lev0);
  v = (val >> pin) & 0x1;
  return v;
}
