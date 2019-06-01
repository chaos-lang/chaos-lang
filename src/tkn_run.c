/* tkn_run.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: Utilities for dealing with token runs.
   */

/* Dependencies */

#include <chaos.h>

/* Allocators & Deallocators */

struct tkn_run *
tkn_run_push(struct tkn_run *tkn_run, struct tkn tkn) {
  assert(tkn_run != NULL);
  assert(tkn_run->len <= CHAOS_TKN_RUN_LEN);

  if (unlikely(tkn_run->len == CHAOS_TKN_RUN_LEN)) {
    tkn_run->next = XCNEW(struct tkn_run);
    tkn_run = tkn_run->next;
  }

  tkn_run->tkns[tkn_run->len++] = tkn;

  return tkn_run;
}

void
tkn_run_free(struct tkn_run *tkn_run) {
  struct tkn_run *prev;
  if (tkn_run) {
    while (tkn_run->next) {
      prev = tkn_run;
      tkn_run = tkn_run->next;
      XDELETE(prev);
    }
    XDELETE(tkn_run);
  }
}
