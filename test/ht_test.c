#include <stdio.h>

#include "xxhash.h"
#include "hash_table.h"

hash_func calc_hash = xxhash32;

int main(void) {
  struct table table;
  struct node *node;
  create_table(&table, 5);
  lookup(&table, "int", 3, INSERT);
  lookup(&table, "double", 6, INSERT);
  lookup(&table, "void", 4, INSERT);
  int i;
  for (i = 0; i < 2; i++) {
    node = lookup(&table, "int", 3, NO_INSERT);
    printf("%s\n", node->str);
    node = lookup(&table, "double", 6, NO_INSERT);
    printf("%s\n", node->str);
    node = lookup(&table, "void", 4, NO_INSERT);
    printf("%s\n", node->str);
  }
  destroy_table(&table);
}
