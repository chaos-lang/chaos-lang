/* lex.c
   author: Simon Lovell Bart (microchips-n-dip)
   description: A new lexer that follows the notions outlined by Sean Barrett
                in his article (http://nothings.org/computer/lexing.html) more
                closely to improve performance.
   date: 2019-06-12
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alias.h"
#include "lex.h"

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

/* Include the lexer tables. */

#include "lex_tables.h"

void lex_table_info(void) {
  printf("Equivalence class table size: %ld\n", sizeof(equivalence_class));
  printf("Lexer table size: %ld\n", sizeof(transitions));
  printf("Increment tables size: %ld\n",
         sizeof(stateful_char_increments) + sizeof(stateful_slice_increments));
}

/* Lex an entire unit. */

void
lex_unit(Unit *unit) {
  enum lexer_state state;
  token *result;
  char c;
  int eq_class;
  while (1) {
    /* Prepare a new token from the tokenrun. */
    result = unit->cur_token++;
    if (unit->cur_token == unit->cur_run->limit) {
      unit->cur_run = next_tokenrun(unit->cur_run);
      unit->cur_token = unit->cur_run->tokens;
    }
    /* Check whether we've reached EOF. */
    if (unlikely(unit->cur == unit->rlimit)) {
      result->type = TOKEN_EOF;
      goto _exit;
    }
    state = LS_START;
    result->len = 0;
    do {
      c = *unit->cur;
      /* Determine the next state to go to. */
      eq_class = equivalence_class[(unsigned char) c];
      state = transitions[state][eq_class];
      result->len += stateful_slice_increments[state];
      /* In the case of increments, we need to reset whenever we've got a
         newline otherwise we'll count empty lines. */
      result->len = (eq_class == EQCLASS_NEWLINE) ? 0 : result->len;
      unit->cur += stateful_char_increments[state];
    } while (state < LS_NONTERMINALS_END);
    result->type = state - LS_NONTERMINALS_END + 1;
#if 0
    if (result->type == TOKEN_ID) {
      /* We need to check for reserved words. */
    }
#endif
  }
_exit:
  /* We're done, do nothing. */
  do {} while (0);
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
