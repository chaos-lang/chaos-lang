#ifndef AST_H
#define AST_H

/* An identifier. */

struct node_identifier {
  struct binding *bind;
};

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

/* Declarations. */

enum declaration_kind {
  DECL_NONE,
  DECL_TYPENAME,
  DECL_VAR
};

struct node_declaration {
  struct node_type *type;
  enum declaration_kind kind;
};

/* Bindings from names to declarations. */

struct binding {
  struct node_identifier *name;
  struct node_declaration *decl;
  struct binding *prev;
};

#define SYMBOL_BINDING(n) ((n)->bind->decl)

/* Scopes. */

struct scope {
  struct binding *bindings;
};

#endif
