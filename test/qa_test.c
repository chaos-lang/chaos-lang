#include <stdio.h>

#include "quick_alloc.h"

int main(void) {
  quick_alloc_init();
  int *a = quick_alloc(128);
  int *b = quick_alloc(128);
  int *c = quick_alloc(2056);
  printf("a = %p\n", a);
  printf("b = %p\n", b);
  printf("c = %p\n", c);
  qa_free_all();
}
