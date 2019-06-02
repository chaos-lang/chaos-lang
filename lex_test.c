#include <stdio.h>

#include "lex.h"

int main(void) {
  const char str[] = "what is up my dudes 42069";
  /* Set up the unit. */
  Unit unit;
  unit.cur = unit.buf = str;
  unit.rlimit = str + sizeof(str);
  _init_tokenrun(&unit.base_run);
  unit.cur_run = &unit.base_run;
  unit.cur_token = unit.base_run.tokens;
  /* Lex the unit. */
  lex_unit(&unit);
  /* Display the output of the lexer. */
  token *tok = unit.base_run.tokens;
  while (tok->type != TOKEN_EOF) {
    printf("%d\n", tok->type);
    tok++;
  }
}
