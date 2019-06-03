/* lex.c
   author: Simon Lovell Bart (microchips-n-dip)
   description: A fairly general-purpose lexer that can handle both our core
                language and DSL.
   date: 2019-05-31
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alias.h"
#include "lex.h"

/* Table of token type associations. */

static const enum token_type
token_assoc[256] = {
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
  ['<']         = TOKEN_LANGLE,
  ['>']         = TOKEN_RANGLE,
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
  ['\n']        = LTKN_NEWLINE
};

/* Initialize a tokenrun. */

void
_init_tokenrun(tokenrun *run) {
  run->next = NULL;
  run->limit = run->tokens + TOKENRUN_SIZE;
}

/* Return the next tokenrun, and allocate it if necessary. */

tokenrun *
next_tokenrun(tokenrun *run) {
  if (run->next == NULL) {
    run->next = XNEW(tokenrun);
    run->next->prev = run;
    _init_tokenrun(run->next);
  }
  return run->next;
}

/* The possible states of the lexer. */

enum lexer_state {
  LS_SCAN,            /* Scan the next token and ascertain its type. */
  LS_NL_DISAMBIG,     /* Newline disambiguation. */
  LS_WHITESPACE,      /* Skip over whitespace. */
  LS_IDENT,           /* Lex over an identifier. */
  LS_NUMBER,          /* Lex over a number. */
  LS_STRING,          /* Lex over a string. */
  LS_DASH_DISAMBIG,   /* Dash disambiguation. */
  LS_LANGLE_DISAMBIG, /* L-angle disambiguation. */
  LS_WHOT,            /* Seriously, whot? */
  LS_LENGTH
};

static const enum lexer_state
transition[LS_LENGTH][TOKEN_LENGTH] = {
  [LS_SCAN][LTKN_NEWLINE]          = LS_NL_DISAMBIG,
  [LS_SCAN][LTKN_WHITESPACE]       = LS_WHITESPACE,
  [LS_SCAN][TOKEN_NAME]            = LS_IDENT,
  [LS_SCAN][TOKEN_NUMBER]          = LS_NUMBER,
  [LS_SCAN][TOKEN_STRING]          = LS_STRING,
  [LS_SCAN][TOKEN_MINUS]           = LS_DASH_DISAMBIG,
  [LS_SCAN][TOKEN_LANGLE]          = LS_LANGLE_DISAMBIG,
  [LS_WHITESPACE][LTKN_WHITESPACE] = LS_WHITESPACE,
  [LS_IDENT][TOKEN_NAME]           = LS_IDENT,
  [LS_IDENT][TOKEN_NUMBER]         = LS_IDENT,
  [LS_NUMBER][TOKEN_NUMBER]        = LS_NUMBER,
  [LS_STRING][TOKEN_STRING]        = LS_SCAN
};

/* Lex an entire unit. */

void
lex_unit(Unit *unit) {
  enum lexer_state state = LS_SCAN;
  enum lexer_state next_state;
  enum token_type type;
  token *result;
  unsigned int i = 0;
  /* Timing. */
  struct timespec tp0, tp1;
  /* Some macros for facilitating accessing characters. */
  #define pc (unit->buf[i - 1])
  #define c  (unit->buf[i])
  #define nc (unit->buf[i + 1])
  /* The main lexer state machine loop. */
  while (1) {
    switch (state) {
      /* Prepare a new token from the current run and if necessary get the
         next run. Also check to see if we've reached the unit's rlimit
         and if so, take appropriate action as that's EOF. */
      /* TODO: Replace `rlimit` with `len`? */
      case LS_SCAN: {
        /*if (tp1.tv_nsec != tp0.tv_nsec)
          timing_stop("Lex stage", tp0, tp1);
        timing_start(tp0);*/
        result = unit->cur_token;
        /* Handle possible EOF. */
        if (unlikely(unit->buf + i == unit->rlimit)) {
          result->type = TOKEN_EOF;
          goto _exit;
        }
        /* Figure out which state to transition to in order to lex the token
           properly. */
        result->type = type = token_assoc[c];
        result->base_index = i;
        result->val.str.len = 0;
        if (type == LTKN_WHITESPACE || type == LTKN_NEWLINE)
          goto _next_state;
        unit->need_eol = 1;
        goto _next_token;
      };
      /* Newline disambiguation. */
      case LS_NL_DISAMBIG: {
        if (unit->need_eol && pc == '\n') {
          result->type = TOKEN_SEMICOLON;
          goto _next_token;
        }
      } goto _next_state;
      /* Lex over some whitespace. */
      case LS_WHITESPACE: {
        type = token_assoc[c];
      } goto _next_state;
      /* Lex over an ident. */
      case LS_IDENT: {
        type = token_assoc[c];
        /* If we encounter a token that isn't a valid ident token, we're done
           lexing the ident, but we still need to run a lookup to see if it's
           a keyword. */
        if (unlikely(type != TOKEN_NAME && type != TOKEN_NUMBER)) {
          result->val.str.base = unit->buf + result->base_index;
          /*lookup_node node = ht_lookup(keywords, result->val.str);
          if (node) {
            result->type = TOKEN_KEYWORD;
            result->val.rid = node->code;
          }*/
        } else
          result->val.str.len++;
      } goto _next_state;
      /* Lex over a number. */
      /* TODO: Actually lex the number maybe? */
      case LS_NUMBER: {
        type = token_assoc[c];
        if (type != TOKEN_NUMBER)
          result->val.str.base = unit->buf + result->base_index;
        else
          result->val.str.len++;
      } goto _next_state;
      /* Lex over a string. */
      case LS_STRING: {
        if (c == '"' && pc != '\\')
          result->val.str.base = unit->buf + result->base_index + 1;
        else
          result->val.str.len++;
        i++;
      } goto _next_state;
      /* Dash disambiguation. */
      case LS_DASH_DISAMBIG: {
        if (c == '>')
          result->type = TOKEN_RARROW;
      } goto _next_state;
      /* L-angle disambiguation. */
      case LS_LANGLE_DISAMBIG: {
        if (c == '-') {
          result->type = TOKEN_LARROW;
          i++;
        }
      } goto _next_state;
      /* Return to scan state. */
      default:
        state = LS_SCAN;
        continue;
    }
    /* Handle incrementing the current token and getting the next tokenrun
       if necessary. */
  _next_token:
    unit->cur_token++;
    if (unlikely(unit->cur_token == unit->cur_run->limit)) {
      unit->cur_run = next_tokenrun(unit->cur_run);
      unit->cur_token = unit->cur_run->tokens;
    }
    /* Handle transitioning to the next state. */
  _next_state:
    asm volatile ("# _next_state start");
    type = token_assoc[c];
    next_state = transition[state][type];
    if (state == next_state || next_state != LS_SCAN)
      i++; /* Increment unless returning to scan for reclassification. */
    state = next_state;
    asm volatile ("# _next_state end");
  }
  /* We're done. */
_exit:
  do {} while (0);
  #undef pc
  #undef c
  #undef nc
}

/* Destroy a file reader unit. */

void
destroy_unit(Unit *unit) {
  /* Just remove the tokenruns for now. */
  tokenrun *run;
  while (unit->cur_run->prev) {
    run = unit->cur_run;
    unit->cur_run = run->prev;
    XDELETE(run);
  }
}
