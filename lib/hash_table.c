#include <stdlib.h>
#include <string.h>

#include "alias.h"
#include "hash_table.h"

void
create_table(struct table *table, unsigned int order) {
  unsigned int nslots = 1 << order;
  table->entries = XCNEWVEC(struct node, nslots);
  table->nslots = nslots;
  table->nentries = 0;
}

void
destroy_table(struct table *table) {
  unsigned int index = 0;
#define NODE (table->entries[index])
  while (index < table->nslots) {
    if (NODE.len > 0)
      XDELETEVEC(NODE.str);
    NODE.len = 0;
    index++;
  }
  free(table->entries);
  table->nentries = 0;
  table->nslots = 0;
#undef NODE
}

static void expand(struct table *);
static struct node *insert(struct table *, unsigned int, const char *,
                           unsigned int);

/* Lookup a hash table node. */

struct node *
lookup(struct table *table, const char *str, unsigned int len,
       enum insert_mode insert_mode) {
  struct node *nodes;
  unsigned int hash;
  unsigned int sizemask;
  unsigned int real_index, index;
  hash = calc_hash(str, len);
  sizemask = table->nslots - 1;
  real_index = index = hash & sizemask;
  nodes = table->entries;
#define NODE        (nodes[index])
#define DELTA(n, i) ((i) - (n).index)
  /* Lookup mechanism stopping conditions: we've encountered a node with
     0 delta, or we've encountered an empty bucket. */
  while (1) {
    if (NODE.hash_value == hash &&
        NODE.len == (unsigned int) len &&
        !memcmp(NODE.str, str, len))
      return nodes + index;
    index++;
    if (DELTA(NODE, index) == 0)
      break;
    if (NODE.len == 0)
      break;
  }
  if (insert_mode == NO_INSERT)
    return 0;
  insert(table, hash, str, len);
  /* Expand if load factor exceeds 0.75. */
  if (++table->nentries * 4 >= table->nslots * 3)
    expand(table);
#undef NODE
#undef DELTA
  return nodes + real_index;
}

/* Insert a new node into the hash table. */

static struct node *
insert(struct table *table, unsigned int hash, const char *str,
       unsigned int len) {
  struct node *nodes, insert;
  unsigned int index, real_index;
  unsigned int sizemask;
  nodes = table->entries;
  sizemask = table->nslots - 1;
  real_index = index = hash & sizemask;
#define NODE        (nodes[index])
#define DELTA(n, i) ((i) - (n).index)
  insert.hash_value = hash;
  insert.index = real_index;
  /* Copy the string to the node. */
  char *chars = XNEWVEC(char, len + 1);
  memcpy(chars, str, len);
  chars[len] = '\0';
  insert.str = (const char *) chars;
  insert.len = len;
  /* Insert mechanism stopping conditions: we've encountered a node with
     DELTA(node, index) < DELTA(insert, index), or we've encountered an empty
     bucket. */
  while (1) {
    if (NODE.len == 0) {
      NODE = insert;
      goto _end;
    }
    if (DELTA(NODE, index) < DELTA(insert, index)) {
      struct node temp = NODE;
      NODE = insert;
      insert = temp;
      break;
    }
    index++;
  }
  /* We need a second stage for the ripple in case we don't find an empty
     bucket to insert into. */
  real_index = index;
  while (1) {
    /* If we encounter an empty bucket, insert and break. */
    if (NODE.len == 0) {
      NODE = insert;
      goto _end;
    }
    /* If we encounter DELTA(node, index) < DELTA(insert, index), swap them
       and continue. */
    if (DELTA(NODE, index) < DELTA(insert, index)) {
      struct node temp = NODE;
      NODE = insert;
      insert = temp;
    }
    index++;
  }
_end:
#undef NODE
#undef DELTA
  return nodes + real_index;
}

/* Expand the hash table when necessary. */

static void
expand(struct table *table) {
  struct table new_table;
  struct node *p, *limit;
  new_table.nslots = table->nslots << 1;
  new_table.entries = XCNEWVEC(struct node, new_table.nslots);
  p = table->entries;
  limit = p + table->nslots;
  do {
    if (p->len > 0)
      insert(&new_table, p->hash_value, p->str, p->len);
  } while (++p < limit);
  free(table->entries);
  table->entries = new_table.entries;
  table->nslots = new_table.nslots;
}
