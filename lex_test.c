#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alias.h"
#include "lex.h"

static const char *
token_name[TOKEN_END] = {
  [TOKEN_OTHER]     = "token_other",
  [TOKEN_NEWLINE]   = "token_newline",
  [TOKEN_NAME]      = "token_name",
  [TOKEN_NUMBER]    = "token_number",
  [TOKEN_EQUALS]    = "token_equals",
  [TOKEN_COLON]     = "token_colon",
  [TOKEN_RARROW]    = "token_rarrow",
  [TOKEN_IMPURE]    = "token_impure",
  [TOKEN_ELLIPSIS]  = "token_ellipsis",
  [TOKEN_OPERATOR]  = "token_operator",
  [TOKEN_EOF]       = "token_eof"
};

void 
unit_read(Unit *unit, char *filename) {
  //assert(unit != NULL);
  //assert(filename != NULL);

  /* Save the filename. */ 
  if (unit->filename == NULL)
    unit->filename = XNEWVEC(char, strlen(filename));
  else
    unit->filename = XRESIZEVEC(char, unit->filename, strlen(filename));
  //assert(unit->filename != NULL);
  strcpy(unit->filename, filename);

  /* Calculate the size of the file. */
  FILE *fp = fopen(filename, "r");
  //assert(fp != NULL);
  fseek(fp, 0, SEEK_END);
  size_t len = (size_t) ftell(fp);
  rewind(fp);

  /* Allocate the source buffer. */
  if (unit->buf == NULL)
    unit->buf = XNEWVEC(char, len + 2);
  else
    unit->buf = XRESIZEVEC(char, unit->buf, len + 2);
  //assert(unit->buf != NULL);
  unit->rlimit = unit->buf + len + 2;

  /* Read the file into the buffer. */
  fread((void *) unit->buf, len, 1, fp);
  fclose(fp);
  ((char *)unit->buf)[len + 1] = '\0';
  ((char *)unit->buf)[len + 2] = '\0';
}

int main(int argc, const char **argv) {
  lex_table_info();
  if (argc < 2) {
    printf("Provide an input file\n");
    exit(1);
  }
  /* Set up the unit. */
  Unit unit;
  unit_read(&unit, argv[1]);
  _init_tokenrun(&unit.base_run);
  unit.base_run.prev = NULL;
  unit.cur_run = &unit.base_run;
  unit.cur_token = unit.base_run.tokens;
  unit.cur = unit.buf;
  /* Lex the unit. */
  struct timespec tp0, tp1;
  timing_start(tp0);
  for (int i = 0; i < 1; i++) {
    unit.cur = unit.buf;
    lex_unit(&unit);
  }
  timing_stop("Lex", tp0, tp1);
  /* Display the output of the lexer. */
  unit.cur_run = &unit.base_run;
  unit.cur_token = unit.cur_run->tokens;
  while (unit.cur_token->type != TOKEN_EOF) {
    //printf("%d\n", unit.cur_token->type);
    if (token_name[unit.cur_token->type])
      printf("%s\n", token_name[unit.cur_token->type]);
    unit.cur_token++;
    if (unit.cur_token == unit.cur_run->limit) {
      unit.cur_run = next_tokenrun(unit.cur_run);
      unit.cur_token = unit.cur_run->tokens;
    }
  }
  destroy_unit(&unit);
}
