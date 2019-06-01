/* lexer.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: The lexer for the compiler.
   date: 2019-05-31
   */

/* Dependencies */

#include <chaos.h>

/* Helper Functions */

static inline struct tkn_run *
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

/* Main Lexer */

static enum tkn_kind tk_classify[256] = {
  ['A' ... 'Z'] = TK_TYPE,
  ['a' ... 'z'] = TK_IDEN,
  ['0' ... '9'] = TK_INT,
  [':'] = TK_OPER,
  ['+'] = TK_OPER,
  ['-'] = TK_DASH,
  ['*'] = TK_OPER,
  ['/'] = TK_OPER,
  ['>'] = TK_OPER,
  ['<'] = TK_OPER
};

void
unit_lex(struct unit *unit) {
  /* Set up a blank token run. */
  if (unit->tkn_run != NULL)
    tkn_run_free(unit->tkn_run);
  unit->tkn_run = XCNEW(struct tkn_run);
  assert(unit->tkn_run);

  /* Prepare some loop variables. */
  struct tkn_run *tkn_run = unit->tkn_run;
  struct tkn tkn = {0};
  size_t i;

  /* Run the source text through a state machine to lex it. */
  for (i = 0; i < unit->src_len; i++) {
    #define c (unit->src[i])
    #define nc (unit->src[i + 1])

    switch (tkn.kind) {
      case TK_NONE: {
        tkn.slice.left = i;
        tkn.kind = tk_classify[c];
        i -= (tkn.kind != TK_NONE) ? 1 : 0;
      } continue;

      case TK_DASH: {
        if (nc == '>') {
          i++;
          tkn.kind = TK_ARROW;
        } else {
          tkn.kind = TK_OPER;
        }
      } continue;

      default: {
        if (tk_classify[c] != tkn.kind)
          break;
      } continue;
    }

    tkn.slice.right = i - 1;
    unit->tkn_run = tkn_run_push(unit->tkn_run, tkn);
    tkn.kind = TK_NONE;

    #undef nc
    #undef c
  }
}

void
unit_lex_print(struct unit *unit) {
  struct tkn_run *tkn_run = unit->tkn_run;
  size_t i;

  while (tkn_run) {
    for (i = 0; i < tkn_run->len; i++) {
      printf("%s ", tk_debug[tkn_run->tkns[i].kind]);
      unit_print_slice(unit, tkn_run->tkns[i].slice);
      putchar('\n');
    }
    tkn_run = tkn_run->next;
  }
}
