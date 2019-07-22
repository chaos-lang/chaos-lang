#ifndef HASH_TABLE_H
#define HASH_TABLE_H

struct node {
  unsigned int hash_value;
  unsigned int index;
  const char *str;
  unsigned int len;
  unsigned int val;
};

struct table {
  struct node *entries;
  unsigned int nslots;
  unsigned int nentries;
};

extern void create_table(struct table *, unsigned int);
extern void destroy_table(struct table *);

typedef unsigned int (*hash_func)(const char *, unsigned int);
extern hash_func calc_hash;

enum insert_mode {
  NO_INSERT,
  INSERT
};

extern struct node *lookup(struct table *, const char *, unsigned int,
                           enum insert_mode);

#endif

