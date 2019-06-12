/* lex.c
   author: Simon Lovell Bart (microchips-n-dip)
   description: A new lexer that follows the notions outlined by Sean Barrett
                in his article (http://nothings.org/computer/lexing.html) more
                closely to improve performance.
   date: 2019-05-31
   */

enum equivalence_classes {
  EQCLASS_WHITESPACE,
  EQCLASS_NEWLINE,
  EQCLASS_LETTER,
  EQCLASS_NUMBER,
  EQCLASS_EQUALS,
  EQCLASS_COLON,
  EQCLASS_MINUS,
  EQCLASS_DOT,
  EQCLASS_GT,
  EQCLASS_OPERATOR
};

static const char
equivalence_class[256] = {
  [' ']         = EQCLASS_WHITESPACE,
  ['\t']        = EQCLASS_WHITESPACE,
  ['\n']        = EQCLASS_NEWLINE,
  ['A' ... 'Z'] = EQCLASS_LETTER,
  ['a' ... 'z'] = EQCLASS_LETTER,
  ['0' ... '9'] = EQCLASS_NUMBER,
  ['=']         = EQCLASS_EQUALS,
  [':']         = EQCLASS_COLON,
  ['+']         = EQCLASS_OPERATOR,
  ['-']         = EQCLASS_MINUS,
  ['*']         = EQCLASS_OPERATOR,
  ['/']         = EQCLASS_OPERATOR,
  ['.']         = EQCLASS_DOT,
  ['>']         = EQCLASS_GT,
  ['<']         = EQCLASS_OPERATOR,
};

enum lexer_state {
  /* Terminal states. */
  LS_NEWLINE_TERMINAL  = -10,
  LS_IDENT_TERMINAL    = -9,
  LS_NUMBER_TERMINAL   = -8,
  LS_EQUALS_TERMINAL   = -7,
  LS_COLON_TERMINAL    = -6,
  LS_RARROW_TERMINAL   = -5,
  LS_IMPURE_TERMINAL   = -4,
  LS_ELLIPSIS_TERMINAL = -3,
  LS_OPERATOR_TERMINAL = -2,
  LS_TERMINALS         = -1,
  LS_START             =  0,
  LS_WHITESPACE,   /* Whitespace, to be skipped. */
  /* Normal textual lexing. */
  LS_IDENT,
  LS_NUMBER,
  LS_STRING,
  LS_CHAR,
  /* Operator lexing states. */
  /* Single char operators. */
  LS_SEEN_EQUALS,   /* "=" */
  LS_SEEN_COLON,    /* ":" */
  LS_SEEN_MINUS,    /* "-" */
  LS_SEEN_RARROW,   /* "->" */
  LS_SEEN_IMPURE,   /* "=>" */
  LS_SEEN_DOT,      /* "." */
  LS_SEEN_DOT_2,    /* ".." */
  /* Triple char operators. */
  LS_SEEN_ELLIPSIS, /* "..." */
  LS_OPERATOR,      /* Generic operator. */
  LS_SEEN_TICK      /* "'" */
};

/* Total number of states. */

static const char LS_LENGTH = LS_SEEN_TICK;

/* State machine transitions table. */

#include "lex_transitions.h"

/* Stateful increments prevent incrementing on terminal states and
   whitespace. */

static const char
stateful_increments[LS_LENGTH] = {
  [LS_START ... LS_LENGTH] = 1
};

void
lex_unit(struct unit *unit) {
  enum lexer_state state;
  token *result;
  char c;
  int eq_class, disambiguation_hash;
  while (1) {
    /* Prepare a new token from the tokenrun. */
    result = unit->cur_token++;
    if (unit->cur_token == unit->cur_run->limit) {
      unit->cur_run = next_tokenrun(unit->cur_run);
      unit->cur_token = unit->cur_run->tokens;
    }
    /* Check whether we've reached EOF. */
    if (unlikely(i == unit->len)) {
      result->type = TOKEN_EOF;
      goto _exit;
    }
    state = LS_START;
    do {
      c = *unit->cur++;
      /* Determine the next state to go to. */
      eq_class = eqivalences[c];
      state = transitions[state][eq_class].state;
      result->len += stateful_increments[state];
    } while (state > LS_TERMINALS);
    result->type = -state;
  }
_exit:
  /* We're done, do nothing. */
  do {} while (0);
}
