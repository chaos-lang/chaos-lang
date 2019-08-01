#include "quick_alloc.h"

#include "lex.h"
#include "ast.h"
#include "parse.h"

#define NEW_NODE(T) (T *) quick_alloc(sizeof(T))

/* Bind a name to the appropriate declaration. */

static struct binding *free_bindings = 0;

void
bind(struct node_identifier *name, struct node_declaration *decl) {
  struct binding *b, **here;
  struct scope _scope, *scope = &_scope; // this is just a placeholder for now.
  /* Allocate space for a new binding. */
  if (free_bindings) {
    b = free_bindings;
    free_bindings = b->prev;
  }
  else
    b = NEW_NODE(struct binding);
  /* Set up the binding in the scope. */
  b->decl = decl;
  b->name = name;
  b->prev = scope->bindings;
  scope->bindings = b;
  /* Properly shadow the bind (I haven't done this yet). */
  here = &name->bind;
  *here = b;
}
