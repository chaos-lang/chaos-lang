/* unit.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: Compilation unit utilities.
   date: 2019-05-31
  */

/* Dependencies */

#include <chaos.h>

/* Allocators & Deallocators */

struct unit
unit_init(void) {
  struct unit unit = {0};
  unit.tkn_run = XCNEW(struct tkn_run);
  return unit;
}

void
unit_free(struct unit *unit) {
  XDELETEVEC(unit->filename);
  XDELETEVEC(unit->src);
  tkn_run_free(unit->tkn_run);
}
