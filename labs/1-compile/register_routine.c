#include <stdio.h>


void foo(void (*f)(int)) {
  (*f)(10);
}

