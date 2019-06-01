/* debug.c 
   authors: Simon Lovell Bart (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: Debug/error info for the compiler.
   date: 2019-05-31
   */

/* Dependencies */

#include <chaos.h>

/* Tables */

char *tk_name[TK_LENGTH] = {
  [TK_NONE] = "none",
  
  [TK_IDEN]    = "variable identifier",
  [TK_TYPE]    = "type identifier",
  [TK_GENERIC] = "generic identifier",

  [TK_OPER] = "operator",
  [TK_ARROW] = "arrow",

  [TK_INT] = "integer literal"
};

char *tk_debug[TK_LENGTH] = {
  [TK_NONE] = "tk_none",

  [TK_IDEN] = "tk_iden",
  [TK_TYPE] = "tk_type",
  [TK_GENERIC] = "tk_generic",

  [TK_OPER] = "tk_oper",
  [TK_ARROW] = "tk_arrow",

  [TK_INT] = "tk_int"
};

char *ak_name[AK_LENGTH] = {
  [AK_NONE] = "none"
};

char *ak_debug[AK_LENGTH] = {
  [AK_NONE] = "ak_none"
};

/* Debug Printing */

void
unit_print_slice(struct unit *unit, struct slice slice) {
  assert(unit != NULL);
  assert(unit->src != NULL);

  size_t i;
  for (i = slice.left; i <= slice.right; i++)
    putchar(unit->src[i]);
}
