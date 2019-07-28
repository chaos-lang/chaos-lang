#include <stdio.h>

#include "quick_alloc.h"

#include "lex.h"
#include "parse.h" /* Remove after testing. */
#include "ast.h"

#if 0

struct parser {
  struct token tokens[2];
  unsigned short num_tokens;
  struct tokenrun *cur_run;
};

#endif

/* Since we lex all tokens before parsing, we can simply use `parser.tokens`
   as a "frame" for looking into the tokenruns, and move the frame along
   incrementally. This may complicate lookahead later on though. */

void
parse_init(struct parser *parser, struct unit *unit) {
  quick_alloc_init();
  parser->num_tokens = 0;
  parser->cur_run = &unit->base_run;
  parser->tokens = parser->cur_run->tokens;
}

static token *
peek_token(struct parser *parser) {
  return &parser->tokens[0];
}

static void
consume_token(struct parser *parser) {
  parser->tokens++;
  if (parser->tokens == parser->cur_run->limit) {
    parser->cur_run = next_tokenrun(parser->cur_run);
    parser->tokens = parser->cur_run->tokens;
  }
}

static int
next_token_is(struct parser *parser, enum token_type type) {
  return peek_token(parser)->type == type;
}

static int
next_token_is_not(struct parser *parser, enum token_type type) {
  return !next_token_is(parser, type);
}

static void
expect_token(struct parser *parser, enum token_type type) {
  if (next_token_is_not(parser, type)) {
    printf("error: expected token %d, got %d\n", type,
           peek_token(parser)->type);
  }
  consume_token(parser);
}

/* Quick alloc a new AST node. */

#define NEW_NODE(T) (T *) quick_alloc(sizeof(T))

struct node_type *parse_primary_type(struct parser *);
struct node_type *parse_type(struct parser *);

/* Parse a primary type as
   <primary-type> -> "(" <type> ")"
   <primary-type> -> <type-primitive>
   <primary-type> -> <type-def>
   */

struct node_type *
parse_primary_type(struct parser *parser) {
  struct node_type *result, *temp;
  if (next_token_is(parser, TOKEN_OPEN_PAREN)) {
    consume_token(parser);
    temp = parse_type(parser);
    if (temp->kind == TYPE_NESTED)
      result = temp;
    else {
      result = NEW_NODE(struct node_type);
      result->val.type = temp;
      result->kind = TYPE_NESTED;
    }
    expect_token(parser, TOKEN_CLOSE_PAREN);
  }
  else if (next_token_is(parser, TOKEN_KEYWORD)) {
    result = NEW_NODE(struct node_type);
    result->kind = TYPE_PRIMITIVE;
    result->val.rid = peek_token(parser)->val.rid;
    consume_token(parser);
  }
  else if (next_token_is(parser, TOKEN_NAME)) {
    result = NEW_NODE(struct node_type);
    result->kind = TYPE_TYPEDEF;
    /* TODO: Add user-defined primary type parsing. */
  }
  else
    printf("error: expected primary type\n");
  return result;
}

/* Parse a type as
   <type> -> <primary-type> "->" <type>
   <type> -> <primary-type>
   */

struct node_type *
parse_type(struct parser *parser) {
  /* Do not create a new node immediately; if we just have a primary type then
     we can reuse the node from that. */
  struct node_type *result;
  result = parse_primary_type(parser);
  /* Now correct for compound types. */
  if (next_token_is(parser, TOKEN_RARROW)) {
    struct node_type *temp = result;
    struct type_list list;
    struct type_list_node *prev;
    result = NEW_NODE(struct node_type);
    result->kind = TYPE_COMPOUND;
    list.head = NEW_NODE(struct type_list_node);
    list.head->type = temp;
    list.tail = list.head;
    while (next_token_is(parser, TOKEN_RARROW)) {
      consume_token(parser);
      prev = list.tail;
      list.tail = NEW_NODE(struct type_list_node);
      list.tail->type = parse_primary_type(parser);
      prev->next = list.tail;
      list.tail->prev = prev;
    }
    result->val.types = list;
  }
  return result;
}

#if 0

struct node_declaration {
  struct node_type *type;
  struct node_identifier_list *identifiers;
};

/* Parse a declaration.
   <declaration> -> <identifier-list> : <primary-type>
   <declaration> -> <identifier-list> : <func-type>
   <func-type> -> <parameter-decl-list> "->" <primary-type>
   <parameter-decl-list> -> <parameter-decl-list> "->" <parameter-decl>
   <parameter-decl-list> -> <parameter-decl>
   <parameter-decl> -> <identifier-list> ":" <primary-type>
   <parameter-decl> -> <primary-type>
   */

struct node_declaration *
parse_declaration(struct parser *parser) {
  
}

#endif
