#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "ast.h"

struct parser {
  struct token *tokens;
  unsigned short num_tokens;
  struct tokenrun *cur_run;
};

extern void parse_init(struct parser *, struct unit *);

extern struct node_type *parse_primary_type(struct parser *);
extern struct node_type *parse_type(struct parser *);

#endif
