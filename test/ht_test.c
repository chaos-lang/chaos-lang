#include <stdio.h>

#include "xxhash.h"
#include "hash_table.h"

enum rid_code {
  RID_INT,
  RID_DOUBLE,
  RID_FLOAT,
  RID_VOID
};

#define HT_RID(n) (*(enum rid_code *) (n)->data)

hash_func calc_hash = xxhash32;

int main(void) {
  struct table table;
  struct node *node;
  create_table(&table, 1);
  char str[5];
  unsigned int i;
  for (i = 0; i < 2000; i++) {
    printf("%d ", i);
    snprintf(str, 5, "%04d", i);
    lookup(&table, str, 4, INSERT);
  }
  printf("\n");
  destroy_table(&table);
}
