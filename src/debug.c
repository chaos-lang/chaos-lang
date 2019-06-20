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
};

char *tk_debug[TK_LENGTH] = {
  [TK_NONE] = "tk_none",

  [TK_IDEN] = "tk_iden",
  [TK_TYPE] = "tk_type",
  
  [TK_OPER] = "tk_oper",

  [TK_COMMA]  = "tk_comma",
  [TK_LBRACK] = "tk_lbrack",
  [TK_RBRACK] = "tk_rbrack",
  [TK_LPAREN] = "tk_lparen",
  [TK_RPAREN] = "tk_rparen",
  [TK_LCURLY] = "tk_lcurly",
  [TK_RCURLY] = "tk_rcurly",

  [TK_NEWL] = "tk_newl",
  
  [TK_EQUALS] = "tk_equals",
  [TK_UNDER]  = "tk_under",
  [TK_DECL]   = "tk_decl",

  [TK_LANGLE] = "tk_langle",
  [TK_RANGLE] = "tk_rangle",
  [TK_DASH]   = "tk_dash",
  [TK_DOT]    = "tk_dot",

  [TK_HASH] = "tk_hash",
  [TK_TICK] = "tk_tick",
  [TK_ESC]  = "tk_esc",
  [TK_QUOT] = "tk_quot",

  [TK_INT] = "tk_int",

  [TK_CHAR_ESCD]             = "tk_char_escd",
  [TK_STRING_ESCD]           = "tk_string_escd",
  [TK_QUOT_QUOT]             = "tk_quot_quot",
  [TK_MULTISTRING_RUN]       = "tk_multistring_run",
  [TK_MULTISTRING_QUOT]      = "tk_multistring_quot",
  [TK_MULTISTRING_QUOT_QUOT] = "tk_multistring_quot_quot",

  [TK_CHAR]        = "tk_char",
  [TK_STRING]      = "tk_string",
  [TK_MULTISTRING] = "tk_multistring",
  [TK_GENERIC]     = "tk_generic",

  [TK_ARROW]    = "tk_arrow",
  [TK_IMPURE]   = "tk_impure",
  [TK_RANGE]    = "tk_range",
  [TK_VARARGS]  = "tk_varargs",
  [TK_RANGE_LT] = "tk_range_lt",

  [TK_LCMT] = "tk_lcmt",
  [TK_RCMT] = "tk_rcmt"
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

  const size_t len = slice.right - slice.left + 1;
  fwrite(unit->src + slice.left, 1, len, stdout);
}
