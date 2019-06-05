/* chaos.h
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: The main header for the Chaos compiler.
   date: 2019-05-31
   */

#ifndef CHAOS_H
#define CHAOS_H

/* Dependencies */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <alias.h>

/* Config */

#define CHAOS_TKN_RUN_LEN 256 /* The static length of a token run. */

/* General Types */

struct slice {
  size_t left;  /* Left boundary, inclusive.  */
  size_t right; /* Right boundary, inclusive. */
};

/* Lexer Types */

enum tkn_kind {
  TK_NONE = 0, /* A placeholder "none" value. */

  TK_IDEN,
  TK_TYPE,
  TK_GENERIC,

  TK_OPER,
  TK_HASH,
  TK_LBRACK,
  TK_RBRACK,
  TK_LPAREN,
  TK_RPAREN,
  TK_LCURLY,
  TK_RCURLY,

  TK_DECL,
  TK_ARROW,

  TK_INT,
  TK_CHAR,

  TK_NEWL,

  TK_LENGTH
};

struct tkn {
  struct slice slice; /* A slice of the source code the token refers to. */
  union {
    unsigned long long u_int; /* A max-size unsigned integer. */
  } val;
  enum tkn_kind kind; /* The kind of the token. */
};

struct tkn_run {
  struct tkn tkns[CHAOS_TKN_RUN_LEN]; /* The set of tokens in the run. */
  size_t len;                         /* The length into the token run. */
  struct tkn_run *next;               /* The pointer to the next run.  */
};

/* Parser Types */

enum ast_kind {
  AK_NONE = 0, /* A placeholder "none" value. */

  AK_LENGTH /* A value representing the length of this enum. */
};

struct ast {
  enum ast_kind kind; /* The kind of the AST node. */
};

/* Unit Type */

struct unit {
  char *filename;          /* The name of the file that the unit represents. */
  char *src;               /* The source text of the unit. */
  size_t src_len;          /* The length of the source text. */
  struct tkn_run *tkn_run; /* The tokenized run. */
  struct ast *ast;         /* The abstract syntax tree. */
};

/* debug.c */

extern char *tk_name[TK_LENGTH];
extern char *tk_debug[TK_LENGTH];

extern char *ak_name[AK_LENGTH];
extern char *ak_debug[AK_LENGTH];

void unit_print_slice(struct unit *unit, struct slice slice);

/* unit.c */

struct unit unit_init(void);
void unit_free(struct unit *unit);

/* reader.c */

void unit_read(struct unit *unit, char *filename);

/* lexer.c */

void tkn_run_free(struct tkn_run *tkn_run);

void unit_lex(struct unit *unit);
void unit_lex_print(struct unit *unit);

#endif /* CHAOS_H */
