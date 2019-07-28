#ifndef AST_H
#define AST_H

/* Kind of type being parsed. */

enum type_kind {
  TYPE_PRIMITIVE,
  TYPE_TYPEDEF,
  TYPE_NESTED,
  TYPE_COMPOUND
};

struct type_list_node {
  struct type_list_node *prev, *next;
  struct node_type *type;
};

struct type_list {
  struct type_list_node *head, *tail;
};

struct node_type {
  enum type_kind kind;
  union {
    enum rid_code rid;
    //struct node_identifier *ident;
    struct node_type *type;
    struct type_list types;
  } val;
};

#define COMPOUND_TYPE_HEAD(t) ((t)->val.types.head)
#define PRIMITIVE_TYPE_RID(t) ((t)->val.rid)

#endif
