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
  TOKEN_EOF,
  LTKN_WHITESPACE      /* This isn't a real token per-se. */
};

/* Table of token type associations. */

static const enum token_type
token_assoc[256] const {
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
  [';']         = TOKEN_SEMICOLON,
  [' ']         = LTKN_WHITESPACE,
  ['\t']        = LTKN_WHITESPACE,
  ['\n']        = LTKN_WHITESPACE
};

typedef struct token token;

struct token {
  enum token_type type;
  unsigned int base_index;
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

/* The possible states of the lexer. */

enum lexer_state {
  LS_SCAN,       /* Scan the next token and ascertain its type. */
  LS_WHITESPACE, /* Skip over whitespace. */
  LS_IDENT,      /* Lex over an identifier. */
  LS_NUMBER,     /* Lex over a number. */
  LS_STRING      /* Lex over a string. */
};

/* Lex an entire unit. */

void
lex_unit(Unit *unit) {
  enum lexer_state state = LS_SCAN;
  enum token_type type;
  token *result;
  unsigned int i;
  /* Some macros for facilitating accessing characters. */
  #define pc (unit->buf[i - 1])
  #define c  (unit->buf[i])
  #define nc (unit->buf[i + 1])
  while (1) {
    switch (state) {
      /* Prepare a new token from the current run and if necessary get the
         next run. Also check to see if we've reached the unit's rlimit
         and if so, take appropriate action as that's EOF. */
      /* TODO: Replace `rlimit` with `len`? */
      case LS_SCAN: {
        result = unit->cur_token;
        /* Handle possible EOF. */
        if (unlikely(unit->buf + i == unit->rlimit)) {
          result->type = TOKEN_EOF;
          goto _exit;
        }
        memset(result, 0, sizeof(token));
        /* Figure out which state to transition to in order to lex the token
           properly. */
        result->type = type = token_assoc[c];
        result->base_index = i;
        switch (type) {
          case LTKN_WHITESPACE: state = LS_WHITESPACE; continue;
          case TOKEN_NAME: state = LS_IDENT; break;
          case TOKEN_NUMBER: state = LS_NUMBER; break;
          case TOKEN_STRING: state = LS_STRING; break;
        }
        /* Handle incrementing the current token and getting the next tokenrun
           if necessary. */
        unit->cur_token++;
        if (unlikely(unit->cur_token == unit->cur_run->limit)) {
          unit->cur_run = next_tokenrun(unit->cur_run);
          unit->cur_token = unit->cur_run->tokens;
        }
        i++;
      } continue;
      /* Lex over some whitespace. */
      case LS_WHITESPACE:
        type = token_assoc[c];
        if (unit->need_eol && pc == '\n')
          result->type = TOKEN_SEMICOLON;
        else if (type != LTKN_WHITESPACE)
          state = LS_SCAN;
        else
          i++;
        continue;
      /* Lex over an ident. */
      case LS_IDENT: {
        type = token_assoc[c];
        /* If we encounter a token that isn't a valid ident token, we're done
           lexing the ident, but we still need to run a lookup to see if it's
           a keyword. */
        if (type != TOKEN_NAME && type != TOKEN_NUMBER) {
          result->val.str.base = unit->buf + result->base_index;
          lookup_node node = ht_lookup(keywords, result->val.str);
          if (node) {
            result->type = TOKEN_KEYWORD;
            result->val.rid = node->code;
          }
          state = LS_SCAN;
        } else {
          result->val.str.len++;
          i++;
        }
      } continue;
      /* Lex over a number. */
      /* TODO: Actually lex the number. */
      case LS_NUMBER: {
        type = token_assoc[c];
        if (type != TOKEN_NUMBER)
          state = LS_SCAN;
        else {
          result->val.str.len++;
          i++;
        }
      } continue;
      /* Lex over a string. */
      /* TODO: Implement this, I got lazy. */
      case LS_STRING: {

      } continue;
    }
  }
_exit:
  /* We're done. */
  #undef pc
  #undef c
  #undef nc
}
