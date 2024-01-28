// write code in C to check if stack grows up or down.
// suggestion:
//   - local variables are on the stack.
//   - so take the address of a local, call another routine, and
//     compare addresses.
//   - make sure you check the machine code make sure the
//     compiler didn't optimize the calls away.
#include "rpi.h"

void func2(int *addr_b) {
    int b = 0;
    *addr_b = (int)&b;
}

void func1(int *addr_a, int *addr_b) {
    int a = 0;
    *addr_a = (int)&a;
    func2(addr_b);
}

int stack_grows_down(void) {
    int addr_a, addr_b;
    func1(&addr_a, &addr_b);
    if (addr_b < addr_a) {
        return 1;
    }
    return 0;
}

void notmain(void) {
    if(stack_grows_down())
        trace("stack grows down\n");
    else
        trace("stack grows up\n");
}
