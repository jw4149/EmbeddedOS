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
#include "gpio.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_set1  = (GPIO_BASE + 0x20),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_clr1  = (GPIO_BASE + 0x2C),
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
  gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
  if (pin >=32 && pin != 47)
    return;
  if (pin <= 31) {
    PUT32(gpio_set0, (1 << pin));
  } else if (pin <= 53) {
    unsigned shift = pin - 32;
    PUT32(gpio_set1, (1 << shift));
  } else {
    return;
  }
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
  if (pin >= 32 && pin != 47)
    return;
  if (pin <= 31) {
    PUT32(gpio_clr0, (1 << pin));
  } else if (pin <= 53) {
    unsigned shift = pin - 32;
    PUT32(gpio_clr1, (1 << shift));
  } else {
    return;
  }
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
  gpio_set_function(pin, GPIO_FUNC_INPUT);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  if (pin >= 32 && pin != 47)
    return -1;
  unsigned v = 0;

  // implement.
  unsigned val = GET32(gpio_lev0);
  v = (val >> pin) & 0x1;
  return DEV_VAL32(v);
}

void gpio_set_function(unsigned pin, gpio_func_t function) {
  if (pin >= 32 && pin != 47) {
    return;
  }
  if (function >= 8) {
    return;
  }
  unsigned reg_n = pin / 10;
  unsigned sel_n = pin % 10;

  unsigned *base_pointer = (void *)GPIO_BASE;
  unsigned addr = (unsigned)(base_pointer + reg_n);

  unsigned val = GET32(addr);

  unsigned mask = 0x7 << (sel_n * 3);
  unsigned shift = 3 * sel_n;

  val &= ~mask;
  val |= (function << shift);
  PUT32(addr, val);
}
