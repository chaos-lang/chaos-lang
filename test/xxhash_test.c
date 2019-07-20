#include <stdio.h>

#include "xxhash.h"

int main(void) {
  unsigned int i, h;
  const char txt0[] = "int";
  const char txt1[] = "double";
  const char txt2[] = "void";
  for (i = 0; i < 5; i++) {
    h = xxhash32((const void *) txt0, 3, 0);
    printf("xxhash32(%s) = %x\n", txt0, h);
    h = xxhash32((const void *) txt1, 6, 0);
    printf("xxhash32(%s) = %x\n", txt1, h);
    h = xxhash32((const void *) txt2, 4, 0);
    printf("xxhash32(%s) = %x\n", txt2, h);
  }
}
