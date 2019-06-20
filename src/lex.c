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

/* Table of disambiguation of tokens.
   We use 0 valued tokens to indicate we just preserve the current token. */

static const enum token_type
disambig_assoc[512] = {
  ['-' + ('>' << 1)] = TOKEN_RARROW,
  ['<' + ('-' << 1)] = TOKEN_LARROW
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
  LS_WHITESPACE,      /* Skip over whitespace. */
  LS_IDENT,           /* Lex over an identifier. */
  LS_NUMBER,          /* Lex over a number. */
  LS_STRING,          /* Lex over a string. */
  LS_DISAMBIG,        /* General disambiguation. */
  LS_WHOT,            /* Seriously, whot? */
  LS_LENGTH
};

static const enum lexer_state
transition[LS_LENGTH][TOKEN_LENGTH] = {
  /* LS_SCAN transitions. */
  [LS_SCAN][LTKN_NEWLINE]          = LS_WHITESPACE,
  [LS_SCAN][LTKN_WHITESPACE]       = LS_WHITESPACE,
  [LS_SCAN][TOKEN_NAME]            = LS_IDENT,
  [LS_SCAN][TOKEN_NUMBER]          = LS_NUMBER,
  [LS_SCAN][TOKEN_STRING]          = LS_STRING,
  [LS_SCAN][TOKEN_MINUS]           = LS_DISAMBIG,
  [LS_SCAN][TOKEN_LANGLE]          = LS_DISAMBIG,
  /* LS_WHITESPACE transitions. */
  [LS_WHITESPACE][LTKN_WHITESPACE] = LS_WHITESPACE,
  /* LS_IDENT transitions. */
  [LS_IDENT][TOKEN_NAME]           = LS_IDENT,
  [LS_IDENT][TOKEN_NUMBER]         = LS_IDENT,
  /* LS_NUMBER transitions. */
  [LS_NUMBER][TOKEN_NUMBER]        = LS_NUMBER,
  /* LS_STRING transitions. */
  [LS_STRING][TOKEN_STRING]        = LS_SCAN
};

/* Lex an entire unit. */

void
lex_unit(Unit *unit) {
  enum lexer_state state = LS_SCAN;
  volatile enum lexer_state next_state;
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
    /* Prepare a new token from the current run and if necessary get the
       next run. Also check to see if we've reached the unit's rlimit
       and if so, take appropriate action as that's EOF. */
    if (state == LS_SCAN) {
      if (unlikely(unit->buf + i == unit->rlimit)) {
        result->type = TOKEN_EOF;
        goto _exit;
      }
      result = unit->cur_token;
      /* Figure out which state to transition to in order to lex the token
           properly. */
      result->type = type = token_assoc[c];
      result->slice_start = i;
      result->slice_end = i;
      /* Handle newlines and whitespace. */
      if (type == LTKN_NEWLINE) {
        if (!unit->need_eol)
          goto _next_state;
        unit->need_eol = 0;
        result->type = TOKEN_SEMICOLON;
      }
      else if (type == LTKN_WHITESPACE)
        goto _next_state;
      else
        unit->need_eol = 1;
      /* Handle incrementing the current token and getting the next tokenrun
         if necessary. */
      unit->cur_token++;
      if (unlikely(unit->cur_token == unit->cur_run->limit)) {
        unit->cur_run = next_tokenrun(unit->cur_run);
        unit->cur_token = unit->cur_run->tokens;
      }
    }
    else if (state == LS_DISAMBIG) {
      /* Look up the next few chars in the disambiguation table. */
      type = disambig_assoc[pc + (c << 1)];
      /* If we find a token, then we can replace the type and skip two chars
         ahead. Otherwise just go ahead one char. */
      if (type != TOKEN_OTHER) {
        result->type = type;
        i++;
      }
    }
    /* Handle transitioning to the next state. */
  _next_state:
    type = token_assoc[c];
    next_state = transition[state][type];
    if (likely(state == next_state || next_state != LS_SCAN)) {
      i++; /* Increment unless returning to scan for reclassification. */
      result->slice_end++;
    }
    state = next_state;
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
