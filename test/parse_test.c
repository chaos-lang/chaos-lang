#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alias.h"
#include "quick_alloc.h"
#include "lex.h"
#include "parse.h"
#include "ast.h"

static const char *
token_name[TOKEN_END] = {
  [TOKEN_OTHER]       = "token_other",
  [TOKEN_NEWLINE]     = "token_newline",
  [TOKEN_KEYWORD]     = "token_keyword",
  [TOKEN_NAME]        = "token_name",
  [TOKEN_NUMBER]      = "token_number",
  [TOKEN_EQUALS]      = "token_equals",
  [TOKEN_COLON]       = "token_colon",
  [TOKEN_OPEN_PAREN]  = "token_open_paren",
  [TOKEN_CLOSE_PAREN] = "token_close_paren",
  [TOKEN_RARROW]      = "token_rarrow",
  [TOKEN_IMPURE]      = "token_impure",
  [TOKEN_ELLIPSIS]    = "token_ellipsis",
  [TOKEN_OPERATOR]    = "token_operator",
  [TOKEN_EOF]         = "token_eof"
};

void 
unit_read(struct unit *unit, const char *filename) {
  /* Save the filename. */ 
  if (unit->filename == NULL)
    unit->filename = XNEWVEC(char, strlen(filename));
  else
    unit->filename = XRESIZEVEC(char, unit->filename, strlen(filename));
  strcpy(unit->filename, filename);
  /* Calculate the size of the file. */
  FILE *fp = fopen(filename, "r");
  fseek(fp, 0, SEEK_END);
  size_t len = (size_t) ftell(fp);
  rewind(fp);
  /* Allocate the source buffer. */
  if (unit->buf == NULL)
    unit->buf = XNEWVEC(char, len + 2);
  else
    unit->buf = XRESIZEVEC(char, unit->buf, len + 2);
  unit->rlimit = unit->buf + len + 2;
  /* Read the file into the buffer. */
  unsigned int count = fread((void *) unit->buf, len, 1, fp);
  fclose(fp);
  ((char *)unit->buf)[len + 1] = '\0';
  ((char *)unit->buf)[len + 2] = '\0';
}

void
print_type(struct node_type *type) {
  switch (type->kind) {
    case TYPE_PRIMITIVE:
      switch (type->val.rid) {
        case RID_INT:    printf("Int");    break;
        case RID_UINT:   printf("Uint");   break;
        case RID_FLOAT:  printf("Float");  break;
        case RID_DOUBLE: printf("Double"); break;
      }
      break;
    case TYPE_NESTED:
      printf("(");
      print_type(type->val.type);
      printf(")");
      break;
    case TYPE_COMPOUND: {
      struct type_list_node *it;
      for (it = COMPOUND_TYPE_HEAD(type); it->next; it = it->next) {
        print_type(it->type);
        printf("->");
      }
      print_type(it->type);
      break;
    }
  }
}

int main(int argc, const char **argv) {
  lex_table_info();
  if (argc < 2) {
    printf("Provide an input file\n");
    exit(1);
  }
  keywords_init();
  /* Set up the unit. */
  struct unit unit;
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
  /* Parse the tokens. */
  struct parser parser;
  parse_init(&parser, &unit);
  struct node_declaration *decl;
  decl = parse_declaration(&parser);
  print_type(decl->type);
  printf("\n");
  qa_free_all();
}
