#include <stdio.h>
#include <time.h>

#include "alias.h"
#include "lex.h"

static const char *
token_name[] = {
  [TOKEN_OTHER]     = "token_other",
  [TOKEN_NAME]      = "token_name",
  [TOKEN_NUMBER]    = "token_number",
  [TOKEN_PLUS]      = "token_plus",
  [TOKEN_MINUS]     = "token_minus",
  [TOKEN_SEMICOLON] = "token_eol",
  [LTKN_WHITESPACE] = "ltkn_whitespace"
};

int main(void) {
  const char str[] =
    "abc+Defg-234\n\
    abc+Defg-234\n\
    abc+Defg-234\n\
    abc+Defg-234\n\
    abc+Defg-234\n\
    abc+Defg-234\n\
    abc+Defg-234\n";
  /* Set up the unit. */
  Unit unit;
  unit.cur = unit.buf = str;
  unit.rlimit = str + sizeof(str);
  _init_tokenrun(&unit.base_run);
  unit.base_run.prev = NULL;
  unit.cur_run = &unit.base_run;
  unit.cur_token = unit.base_run.tokens;
  /* Lex the unit. */
  struct timespec tp0, tp1;
  timing_start(tp0);
  lex_unit(&unit);
  timing_stop("Lex", tp0, tp1);
  /* Display the output of the lexer. */
  token *tok = unit.base_run.tokens;
  while (tok->type != TOKEN_EOF) {
    printf("%s\n", token_name[tok->type]);
    tok++;
  }
  destroy_unit(&unit);
}
