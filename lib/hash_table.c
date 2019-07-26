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

static inline void expand(struct table *);
static inline struct node *insert(struct table *, unsigned int, const char *,
                           unsigned int);
static inline struct node *transfer(struct table *, struct node *);

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
        !memcmp(NODE.str, str, len)) {
      while (0) ;
      return nodes + index;
    }
    index++;
    if (DELTA(NODE, index) == 0)
      break;
    if (NODE.len == 0)
      break;
  }
  if (insert_mode == NO_INSERT)
    return 0;
  /* Expand if load factor exceeds 0.75. */
  if (++table->nentries * 4 >= table->nslots * 3)
    expand(table);
#undef NODE
#undef DELTA
  return insert(table, hash, str, len);
}

/* Insert a new node into the hash table. */

static inline struct node *
insert(struct table *table, unsigned int hash, const char *str,
       unsigned int len) {
  struct node *nodes, insert, temp;
  unsigned int index, real_index;
  unsigned int sizemask;
  nodes = table->entries;
  sizemask = table->nslots - 1;
  real_index = index = hash & sizemask;
#define NODE1       (nodes[real_index])
#define NODE2       (nodes[index])
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
    if (NODE1.len == 0) {
      memcpy(&NODE1, &insert, sizeof(struct node));
      goto _end;
    }
    if (DELTA(NODE1, index) < DELTA(insert, index)) {
      memcpy(&temp, &NODE1, sizeof(struct node));
      memcpy(&NODE1, &insert, sizeof(struct node));
      memcpy(&insert, &temp, sizeof(struct node));
      break;
    }
    real_index = (real_index + 1) & sizemask;
  }
  index = real_index;
  /* We need a second stage for the ripple in case we don't find an empty
     bucket to insert into. */
  while (1) {
    /* If we encounter an empty bucket, insert and break. */
    if (NODE2.len == 0) {
      memcpy(&NODE2, &insert, sizeof(struct node));
      goto _end;
    }
    /* If we encounter DELTA(node, index) < DELTA(insert, index), swap them
       and continue. */
    if (DELTA(NODE2, index) < DELTA(insert, index)) {
      memcpy(&temp, &NODE2, sizeof(struct node));
      memcpy(&NODE2, &insert, sizeof(struct node));
      memcpy(&insert, &temp, sizeof(struct node));
    }
    index = (index + 1) & sizemask;
  }
_end:
#undef NODE1
#undef NODE2
#undef DELTA
  return nodes + real_index;
}

/* Transfer a node (copy its contents). */

static inline struct node *
transfer(struct table *table, struct node *insert) {
  struct node *nodes, temp, temp2;
  unsigned int index, real_index;
  unsigned int sizemask;
  nodes = table->entries;
  sizemask = table->nslots - 1;
  real_index = index = insert->hash_value & sizemask;
#define NODE1       (nodes[real_index])
#define NODE2       (nodes[index])
#define DELTA(n, i) ((i) - (n).index)
  insert->index = real_index;
  /* From here the process is effectively identical to `insert()`, with the
     only difference being that we use `temp` differently to avoid mucking
     about with the contents of `insert`. */
  while (1) {
    if (NODE1.len == 0) {
      memcpy(&NODE1, insert, sizeof(struct node));
      goto _end;
    }
    if (DELTA(NODE1, real_index) < DELTA(*insert, real_index)) {
      memcpy(&temp, &NODE1, sizeof(struct node));
      memcpy(&NODE1, insert, sizeof(struct node));
      break;
    }
    real_index = (real_index + 1) & sizemask;
  }
  index = real_index;
  while (1) {
    if (NODE2.len == 0) {
      memcpy(&NODE2, &temp, sizeof(struct node));
      goto _end;
    }
    if (DELTA(NODE2, index) < DELTA(temp, index)) {
      memcpy(&temp2, &NODE2, sizeof(struct node));
      memcpy(&NODE2, &temp, sizeof(struct node));
      memcpy(&temp, &temp2, sizeof(struct node));
    }
    index = (index + 1) & sizemask;
  }
_end:
#undef NODE1
#undef NODE2
#undef DELTA
  return nodes + real_index;
}

/* Expand the hash table when necessary. */

static inline void
expand(struct table *table) {
  struct table new_table;
  struct node *p, *limit;
  new_table.nslots = table->nslots << 1;
  new_table.entries = XCNEWVEC(struct node, new_table.nslots);
  p = table->entries;
  limit = p + table->nslots;
  do {
    if (p->len > 0)
      transfer(&new_table, p);
  } while (++p < limit);
  free(table->entries);
  table->entries = new_table.entries;
  table->nslots = new_table.nslots;
}

