/* lexer.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: The lexer for the compiler.
   date: 2019-05-31
   */

/* Dependencies */

#include <chaos.h>

/* Helper Functions */

char tk_oper_classify[512] = {
  ['-' + '>'] = TK_ARROW
};

char tk_oper_check[256] = {
  ['-'] = TK_ARROW
};

static inline struct tkn_run *
tkn_run_push(struct tkn_run *tkn_run, struct tkn tkn) {
  assert(tkn_run != NULL);
  assert(tkn_run->len <= CHAOS_TKN_RUN_LEN);

  tkn.kind = (tkn.kind == TK_OPER)

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

char tk_classify[256] = {
  ['a' ... 'z'] = TK_IDEN,
  ['A' ... 'Z'] = TK_TYPE,
  
  ['+'] = TK_OPER,
  ['-'] = TK_OPER,
  ['*'] = TK_OPER,
  ['/'] = TK_OPER,
  ['<'] = TK_OPER,
  ['>'] = TK_OPER,
  ['#'] = TK_HASH,
  ['['] = TK_LBRACK,
  [']'] = TK_RBRACK,
  ['('] = TK_LPAREN,
  [')'] = TK_RPAREN,
  ['{'] = TK_LCURLY,
  ['}'] = TK_RCURLY,

  [':'] = TK_DECL,

  ['0' ... '9'] = TK_INT,

  ['\n'] = TK_NEWL
};

char tk_transition[TK_LENGTH][TK_LENGTH] = {
  /* None -> State */
  [TK_NONE][TK_IDEN] = TK_IDEN,
  [TK_NONE][TK_TYPE] = TK_TYPE,

  [TK_NONE][TK_OPER] = TK_OPER,
  [TK_NONE][TK_HASH] = TK_HASH,
  [TK_NONE][TK_LBRACK] = TK_LBRACK,
  [TK_NONE][TK_RBRACK] = TK_RBRACK,
  [TK_NONE][TK_LPAREN] = TK_LPAREN,
  [TK_NONE][TK_RPAREN] = TK_RPAREN,
  [TK_NONE][TK_LCURLY] = TK_LCURLY,
  [TK_NONE][TK_RCURLY] = TK_RCURLY,

  [TK_NONE][TK_DECL] = TK_DECL,

  [TK_NONE][TK_INT] = TK_INT,

  [TK_NONE][TK_NEWL] = TK_NEWL,

  /* Codes */
  [TK_IDEN][TK_IDEN] = TK_IDEN,
  [TK_IDEN][TK_TYPE] = TK_IDEN,

  [TK_TYPE][TK_IDEN] = TK_TYPE,
  [TK_TYPE][TK_TYPE] = TK_TYPE,

  [TK_OPER][TK_OPER] = TK_OPER
};

void
unit_lex(struct unit *unit) {
  /* Set up a blank token run. */
  if (unit->tkn_run != NULL)
    tkn_run_free(unit->tkn_run);
  unit->tkn_run = XCNEW(struct tkn_run);
  assert(unit->tkn_run);

  struct tkn_run *tkn_run = unit->tkn_run;

  size_t i;
  struct tkn tkn = {0};
  for (i = 0; i < unit->src_len; i++) {
    enum tkn_kind new_kind = tk_transition[tkn.kind][tk_classify[unit->src[i]]];
    new_kind = new_kind ? new_kind : tk_transition[TK_NONE][tk_classify[unit->src[i]]];
    if (new_kind != tkn.kind) {
      tkn.slice.left = tkn.slice.right + 1;
      tkn.slice.right = i - 1;
      if (unlikely(tkn.kind != TK_NONE))
        tkn_run = tkn_run_push(tkn_run, tkn);
    }
    tkn.kind = new_kind;
  }
  tkn.slice.left = tkn.slice.right + 1;
  tkn.slice.right = i - 1;
  tkn_run_push(tkn_run, tkn);
}

void
unit_lex_print(struct unit *unit) {
  struct tkn_run *tkn_run = unit->tkn_run;
  size_t i;

  while (tkn_run) {
    for (i = 0; i < tkn_run->len; i++) {
      printf("%s ", tk_debug[tkn_run->tkns[i].kind]);
      if (tkn_run->tkns[i].kind != TK_NEWL)
        unit_print_slice(unit, tkn_run->tkns[i].slice);
      putchar('\n');
    }
    tkn_run = tkn_run->next;
  }
}
