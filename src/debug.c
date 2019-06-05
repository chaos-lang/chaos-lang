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

  [TK_IDEN] = "identifier",
  [TK_TYPE] = "type",
  [TK_GENERIC] = "generic",
  
  [TK_OPER] = "operator",
  [TK_HASH] = "hash `#`",
  [TK_LBRACK] = "left square bracket `[`",
  [TK_RBRACK] = "right square bracket `]`",
  [TK_LPAREN] = "left parenthesis `(`",
  [TK_RPAREN] = "right parenthesis `)`",
  [TK_LCURLY] = "left curly bracket `{`",
  [TK_RCURLY] = "right curly bracket `}`",
 
  [TK_DECL]  = "declaration `:`",
  [TK_ARROW] = "arrow `->`",

  [TK_INT] = "integer literal",
  [TK_CHAR] = "character literal",

  [TK_NEWL] = "newline"
};

char *tk_debug[TK_LENGTH] = {
  [TK_NONE] = "tk_none",

  [TK_IDEN] = "tk_iden",
  [TK_TYPE] = "tk_type",
  [TK_GENERIC] = "tk_generic",
  
  [TK_OPER] = "tk_oper",
  [TK_HASH] = "tk_hash",
  [TK_LBRACK] = "tk_lbrack",
  [TK_RBRACK] = "tk_rbrack",
  [TK_LPAREN] = "tk_lparen",
  [TK_RPAREN] = "tk_rparen",
  [TK_LCURLY] = "tk_lcurly",
  [TK_RCURLY] = "tk_rcurly",

  [TK_DECL]  = "tk_decl",
  [TK_ARROW] = "tk_arrow",

  [TK_INT] = "tk_int",
  [TK_CHAR] = "tk_char",

  [TK_NEWL] = "tk_newl"
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
