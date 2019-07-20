#include <stdio.h>

#include "xxhash.h"

int main(void) {
  unsigned int i, h;
  const char txt0[] = "int";
  const char txt1[] = "double";
  const char txt2[] = "void";
  for (i = 0; i < 5; i++) {
    h = xxhash32((const void *) txt0, sizeof txt0, 0);
    printf("xxhash32(%s) = %u\n", txt0, h);
    h = xxhash32((const void *) txt1, sizeof txt1, 0);
    printf("xxhash32(%s) = %u\n", txt1, h);
    h = xxhash32((const void *) txt2, sizeof txt2, 0);
    printf("xxhash32(%s) = %u\n", txt2, h);
  }
}
