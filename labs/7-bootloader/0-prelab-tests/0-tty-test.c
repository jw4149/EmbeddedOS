// note: you can't do a make check for this currently since the name
// will be different on different computers.
#include "libunix.h"
int main(void) {
    
    char *tty;
    while(!(tty = find_ttyusb()))
        output("no tty detected: plug in\n");

    output("tty=<%s>: (should be an absolute path)\n", tty);
    return 0;
}
