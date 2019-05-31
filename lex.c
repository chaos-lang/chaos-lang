/* lex.c
   author: Simon Lovell Bart (microchips-n-dip)
   description: A fairly general-purpose lexer that can handle both our core
                language and DSL.
   date: 2019-05-31
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alias.h"

/* Tokens. */

/* Token type enumeration. */

enum token_type {
  TOKEN_OTHER,         /* An unknown character. */
  TOKEN_NAME,          /* Name/identifier token. */
  TOKEN_KEYWORD,       /* Keyword/reserved word token. */
  TOKEN_NUMBER,        /* A number token. */
  TOKEN_STRING,        /* A string literal token. */
  TOKEN_CHAR,
  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACKET,
  TOKEN_OPEN_BRACE,
  TOKEN_CLOSE_BRACE,
  TOKEN_EQUALS,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MULT,
  TOKEN_DIV,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_COMMENT,
  TOKEN_SEMICOLON,
  TOKEN_EOF
};

/* Table of token type associations. */

static const enum token_type token_assoc[] const {
  ['_']         = TOKEN_NAME,
  ['A' ... 'Z'] = TOKEN_NAME,
  ['a' ... 'z'] = TOKEN_NAME,
  ['0' ... '9'] = TOKEN_NUMBER,
  ['"']         = TOKEN_STRING,
  ['\'']        = TOKEN_CHAR,
  ['(']         = TOKEN_OPEN_PAREN,
  [')']         = TOKEN_CLOSE_PAREN,
  ['[']         = TOKEN_OPEN_BRACKET,
  [']']         = TOKEN_CLOSE_BRACKET,
  ['{']         = TOKEN_OPEN_BRACE,
  ['}']         = TOKEN_CLOSE_BRACE,
  ['=']         = TOKEN_EQUALS,
  ['+']         = TOKEN_PLUS,
  ['-']         = TOKEN_MINUS,
  ['*']         = TOKEN_MULT,
  ['/']         = TOKEN_DIV,
  [':']         = TOKEN_COLON,
  [',']         = TOKEN_COMMA,
  [';']         = TOKEN_SEMICOLON
};

typedef struct token token;

struct token {
  enum token_type type;
  union {
    int integer;
    String str;
    enum rid_code rid;
  } val;
};

/* Tokenruns. */

/* Tokenruns are sized statically rather than dynamically so that, at the cost
   of being able to change their size, we can allocate them in one go rather
   than two. */

#define TOKENRUN_SIZE 256

typedef struct tokenrun tokenrun

struct tokenrun {
  tokenrun *prev, *next;
  token tokens[TOKENRUN_SIZE];
  token *limit;
};

/* Return the next tokenrun, and allocate it if necessary. */

tokenrun *
next_tokenrun(tokenrun *run) {
  tokenrun *next = run->next;
  if (next == NULL) {
    next = XNEW(tokenrun);
    next->prev = run;
    next->next = NULL;
    next->limit = next->tokens + TOKENRUN_SIZE;
  }
  return next;
}

/* A lexer unit; handles a single file. */

typedef struct Unit Unit;

struct Unit {
  /* Current position in the buffer. */
  const char *cur;
  /* The buffer itself. */
  const char *buf;
  /* The buffer limit, used for detecting EOF. */
  const char *rlimit;
  /* Flag indicating whether to replace the next newline with an EOL. */
  unsigned int need_eol : 1;
  unit *prev;
  /* Tokenruns (put in the outer reader struct?) */
  tokenrun base_run;
  tokenrun *cur_run;
  token *cur_token;
};

/* Lex an entire unit. */

void
lex_unit(Unit *unit) {
  token *result;
  char c;
  enum token_type type;
  while (1) {
    result = unit->cur_token++;
    /* If we've reached the limit of the current tokenrun, go to the next
       one and reset our position. */
    if (unit->cur_token == run->limit) {
      unit->cur_run = next_tokenrun();
      unit->cur_token = unit->cur_run->tokens;
    }
  _skipped_whitespace:
    c = *unit->cur++;
    result->type = type = token_assoc[c];
    /* Handle newlines and whitespace. If this is a newline then set need_eol
       to false and we can treat the rest as whitespace. */
    if (c == '\n' && unit->need_eol) {
      unit->need_eol = 0;
      result->type = TOKEN_SEMICOLON;
      continue;
    } else if (c == ' ' || c == '\t' || c == '\n') {
      while (c == ' ' || c == '\t' || c == '\n')
        c = *unit->cur++;
      goto _skipped_whitespace;
    }
    /* For now assume that any other tokens will need eol after them. */
    unit->need_eol = 1;
    switch (result->type) {
      /* Get the full name and put it in a string, then check to see if it's a
         keyword. */
      case TOKEN_NAME: {
        result->val.str.base = unit->cur - 1;
        while (type == TOKEN_NAME || type == TOKEN_NUMBER) {
          c = *unit->cur++;
          type = token_assoc[c];
        }
        result->val.str.len = unit->cur - result->val.str.base;
        lookup_node node = ht_lookup(keywords, result->val.str);
        if (node) {
          result->type = TOKEN_KEYWORD;
          result->val.rid = node->code;
        }
      } continue;
      /* Read the numerical value of a number token.
         TODO: Actually turn it into a numerical value. */
      case TOKEN_NUMBER: {
        result->val.str.base = unit->cur - 1;
        while (type == TOKEN_NUMBER) {
          c = *unit->cur++;
          type = token_assoc[c];
        }
        result->val.str.len = unit->cur - result->val.str.base;
      } continue;
      /* Deal with ambiguity of '/'. */
      case TOKEN_DIV: {
        /* Skip over comments; pretend they're whitespace for now. */
        if (*unit->cur == '/') { /* Line comment, skip to next newline. */
          do {
            c = *unit->cur++;
          } while (c != '\n');
          goto _skipped_whitespace;
        }
        /* Block comment. Look for the next '/' and check if the previous
           character was '*'. */
        else if (*unit->cur == '*') {
          while (1) {
            c = *unit->cur++;
            if (c == '/' && unit->cur[-1] == '*')
              break;
          }
          goto _skipped_whitespace;
        }
      } continue;
    }
  }
}
