#include "rpi.h"
#include "foo.h"

void notmain(void) {
    printk("PI:about to call foo()\n");
    foo();
    clean_reboot();
}
