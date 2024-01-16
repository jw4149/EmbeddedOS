#include <stdlib.h>

int *foo() {
  int *p = (int *)malloc(4);
  *p = 1;
  return p;
}
