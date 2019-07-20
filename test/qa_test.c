#include <stdio.h>

#include "quick_alloc.h"

int main(void) {
  quick_alloc_init();
  int *a = quick_alloc(128 * sizeof(int));
  int *b = quick_alloc(128 * sizeof(int));
  int *c = quick_alloc(2056 * sizeof(int));
  int i;
  for (i = 0; i < 128; i++)
    a[i] = i;
  for (i = 0; i < 128; i++)
    b[i] = i + 128;
  for (i = 0; i < 2056; i++)
    c[i] = i + 256;
  for (i = 0; i < 128; i++)
    printf("%d ", b[i]);
  printf("\n");
  for (i = 128; i < 256; i++)
    printf("%d ", c[i]);
  printf("\n\n");
  for (i = 0; i < 256; i++)
    printf("%d ", a[i]);
  printf("\n");
  qa_free_all();
}
