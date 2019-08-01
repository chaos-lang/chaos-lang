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

/* We need to do name classification here since we can't do it during
   lexing. */

static token *
peek_token(struct parser *parser) {
  struct token *result = &parser->tokens[0];
  if (result->type == TOKEN_NAME) {
    struct node_identifier *id = result->val.ident;
    if (id->bind) {
      if (id->bind->decl->kind == DECL_TYPENAME)
        result->id_kind = ID_TYPENAME;
      else
        result->id_kind = ID_ID;
    }
  }
  return result;
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

static int
next_token_is_identifier(struct parser *parser) {
  return next_token_is(parser, TOKEN_NAME) &&
         (peek_token(parser)->id_kind == ID_ID);
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

/* Parse a parameter declaration.
   <parameter-decl> -> <identifier-list> ":" <primary-type>
   <parameter-decl> -> <primary-type>
   */

struct node_declaration *
parse_parameter_decl(struct parser *parser) {
  struct node_declaration *result;
  int seen_names = 0;
  result = NEW_NODE(struct node_declaration);
  /* Bind the identifier list. */
  /* TODO: We'll need to bind any names in scope. */
  while (next_token_is(parser, TOKEN_NAME)) {
    bind(peek_token(parser)->val.ident, result);
    consume_token(parser);
    seen_names = 1;
  }
  /* We should only expect a colon if we've seen names; otherwise we can go
     directly to parsing the type. */
  if (seen_names)
    expect_token(parser, TOKEN_COLON);
  /* Now we parse the type. */
  result->type = parse_primary_type(parser);
  return result;
}

/* Parse a declaration.
   <declaration> -> <identifier-list> : <primary-type>
   <declaration> -> <identifier-list> : <func-type>
   <func-type> -> <parameter-decl-list> "->" <primary-type>
   <parameter-decl-list> -> <parameter-decl-list> "->" <parameter-decl>
   <parameter-decl-list> -> <parameter-decl>
   */

struct node_declaration *
parse_declaration(struct parser *parser) {
  struct node_declaration *result = NEW_NODE(struct node_declaration);
  /* Bind identifiers. */
  while (next_token_is(parser, TOKEN_NAME)) {
    bind(peek_token(parser)->val.ident, result);
    consume_token(parser);
  }
  expect_token(parser, TOKEN_COLON);
  /* Parse our first parameter, be prepared to promote the type to a list if
     we encounter an RARROW. */
  struct node_declaration *param = parse_parameter_decl(parser);
  if (next_token_is(parser, TOKEN_RARROW)) {
    /* Create a compound type, as in parse_type(), but with
       parse_parameter_decl() called. */
    result->type = NEW_NODE(struct node_type);
    result->type->kind = TYPE_COMPOUND;
    struct type_list list;
    struct type_list_node *prev;
    list.head = NEW_NODE(struct type_list_node);
    list.head->type = param->type;
    list.tail = list.head;
    while (next_token_is(parser, TOKEN_RARROW)) {
      consume_token(parser);
      prev = list.tail;
      list.tail = NEW_NODE(struct type_list_node);
      param = parse_parameter_decl(parser);
      list.tail->type = param->type;
      prev->next = list.tail;
      list.tail->prev = prev;
    }
    result->type->val.types = list;
  }
  /* There was only one parameter and it's the type. */
  else
    result->type = param->type;
  return result;
}


