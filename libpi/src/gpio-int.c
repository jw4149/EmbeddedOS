// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"

enum {
    INT_REG_BASE = 0x2000B000,
    irq_pending2 = (INT_REG_BASE + 0x208),
    enable_irqs2 = (INT_REG_BASE + 0x214),
};

enum {
    GPIO_BASE = 0x20200000,
    GPEDS0 = (GPIO_BASE + 0x40),
    GPREN0 = (GPIO_BASE + 0x4C),
    GPFEN0 = (GPIO_BASE + 0x58),
};

// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) {
    unsigned val = GET32(irq_pending2);
    return DEV_VAL32((val >> 17) & 0x1);
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    // todo("implement: detect rising edge\n");
    if (pin >= 32) {
        return;
    }
    dev_barrier();
    unsigned val = GET32(GPREN0);
    val |= (1 << pin);
    PUT32(GPREN0, val);
    dev_barrier();
    PUT32(enable_irqs2, (1 << 17));
    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    // todo("implement: detect falling edge\n");
    if (pin >= 32) {
        return;
    }
    dev_barrier();
    unsigned val = GET32(GPFEN0);
    val |= (1 << pin);
    PUT32(GPFEN0, val);
    dev_barrier();
    PUT32(enable_irqs2, (1 << 17));
    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    // todo("implement: is an event detected?\n");
    if (pin >= 32) {
        return 0;
    }
    dev_barrier();
    unsigned val = GET32(GPEDS0);
    dev_barrier();
    return DEV_VAL32((val >> pin) & 0x1);
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    // todo("implement: clear event on <pin>\n");
    if (pin >= 32) {
        return;
    }
    dev_barrier();
    PUT32(GPEDS0, (1 << pin));
    dev_barrier();
}
