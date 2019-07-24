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
  node = lookup(&table, "int", 3, INSERT);
  HT_RID(node) = RID_INT;
  node = lookup(&table, "double", 6, INSERT);
  HT_RID(node) = RID_DOUBLE;
  node = lookup(&table, "void", 4, INSERT);
  HT_RID(node) = RID_VOID;
  int i;
  for (i = 0; i < 2; i++) {
    node = lookup(&table, "int", 3, NO_INSERT);
    printf("%s => %d\n", node->str, HT_RID(node));
    node = lookup(&table, "double", 6, NO_INSERT);
    printf("%s => %d\n", node->str, HT_RID(node));
    node = lookup(&table, "void", 4, NO_INSERT);
    printf("%s => %d\n", node->str, HT_RID(node));
  }
  node = lookup(&table, "float", 5, INSERT);
  HT_RID(node) = RID_FLOAT;
  printf("%s => %d\n", node->str, HT_RID(node));
  destroy_table(&table);
}
