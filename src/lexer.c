/* lexer.c
   authors: Simon Lovell Bart      (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: The lexer for the compiler.
   date: 2019-05-31
   */

/* Dependencies */

#include <chaos.h>
#include <time.h>

/* Globals */

static struct {
  long long cmt_depth; // #[ comment ]#
  char      cmt_hash;  // # comment
  char      indent;
} push_state = {0};

static inline struct tkn_run *
tkn_run_push_raw(struct unit *restrict unit, struct tkn_run *restrict tkn_run,
struct tkn tkn) {
  assert(tkn_run != NULL);
  assert(tkn_run->len <= CHAOS_TKN_RUN_LEN);

  if (unlikely(tkn_run->len == CHAOS_TKN_RUN_LEN)) {
    tkn_run->next = XCNEW(struct tkn_run);
    tkn_run = tkn_run->next;
  }

  tkn_run->tkns[tkn_run->len++] = tkn;

  return tkn_run;
}

static inline struct tkn_run *
tkn_run_push(struct unit *restrict unit, struct tkn_run *restrict tkn_run,
struct tkn tkn) {
  assert(tkn_run != NULL);
  assert(tkn_run->len <= CHAOS_TKN_RUN_LEN);

  push_state.cmt_depth += (tkn.kind == TK_LCMT) ? 1 
                        : (tkn.kind == TK_RCMT) ? -1
                        : 0; 
  
  push_state.cmt_hash = (tkn.kind == TK_HASH) ? 1 
                      : (tkn.kind == TK_NEWL) ? 0 
                      : push_state.cmt_hash;

  tkn.kind = (push_state.cmt_hash || push_state.cmt_depth) ? TK_NONE : tkn.kind;

  switch (tkn.kind) {
    case TK_RCMT:
    case TK_NONE: {
      return tkn_run;
    } break;

    case TK_NEWL: {
      push_state.indent = tkn.slice.right - tkn.slice.left;
      for (size_t i = tkn.slice.left + 1; i <= tkn.slice.right; i++)
        push_state.indent -= (unit->src[i] == '\n') ? 1 : 0;
    } break;

    case TK_INT: {
      tkn.val.uint = unit_slice_atoi(unit, tkn.slice);
    } break;
  }

  tkn.indent = push_state.indent;
  tkn_run = tkn_run_push_raw(unit, tkn_run, tkn);

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

static char tk_classify[256] = {
  ['a' ... 'z'] = TK_IDEN,
  ['A' ... 'Z'] = TK_TYPE,

  ['+'] = TK_OPER,
  ['*'] = TK_OPER,
  ['/'] = TK_OPER,

  [','] = TK_COMMA,
  ['['] = TK_LBRACK,
  [']'] = TK_RBRACK,
  ['('] = TK_LPAREN,
  [')'] = TK_RPAREN,
  ['{'] = TK_LCURLY,
  ['}'] = TK_RCURLY,

  ['\n'] = TK_NEWL,

  ['='] = TK_EQUALS,
  ['_'] = TK_UNDER,
  [':'] = TK_DECL,

  ['<'] = TK_LANGLE,
  ['>'] = TK_RANGLE,
  ['-'] = TK_DASH,
  ['.'] = TK_DOT,

  ['#'] = TK_HASH,

  ['0' ... '9'] = TK_INT,
};

static char tk_transition[TK_LENGTH][TK_TRANSITION] = {
  /* Initial States */
  [TK_NONE][TK_IDEN]   = TK_IDEN,
  [TK_NONE][TK_TYPE]   = TK_TYPE,
  [TK_NONE][TK_OPER]   = TK_OPER,
  
  [TK_NONE][TK_COMMA]  = TK_COMMA,
  [TK_NONE][TK_LBRACK] = TK_LBRACK,
  [TK_NONE][TK_RBRACK] = TK_RBRACK,
  [TK_NONE][TK_LPAREN] = TK_LPAREN,
  [TK_NONE][TK_RPAREN] = TK_RPAREN,
  [TK_NONE][TK_LCURLY] = TK_LCURLY,
  [TK_NONE][TK_RCURLY] = TK_RCURLY,

  [TK_NONE][TK_NEWL] = TK_NEWL,

  [TK_NONE][TK_EQUALS] = TK_EQUALS,
  [TK_NONE][TK_UNDER]  = TK_UNDER,
  [TK_NONE][TK_DECL]   = TK_DECL,

  [TK_NONE][TK_LANGLE] = TK_LANGLE,
  [TK_NONE][TK_RANGLE] = TK_RANGLE,
  [TK_NONE][TK_DASH]   = TK_DASH,
  [TK_NONE][TK_DOT]    = TK_DOT,

  [TK_NONE][TK_HASH] = TK_HASH,

  [TK_NONE][TK_INT] = TK_INT,

  /* Generic Kinds Propagation */
  [TK_IDEN][TK_IDEN] = TK_IDEN,
  [TK_IDEN][TK_TYPE] = TK_IDEN,

  [TK_TYPE][TK_IDEN] = TK_TYPE,
  [TK_TYPE][TK_TYPE] = TK_TYPE,

  [TK_OPER][TK_OPER] = TK_OPER,
  [TK_OPER][TK_EQUALS ... TK_DOT] = TK_OPER,

  /* (Sovereign) Disambiguation Operators Propagation */
  [TK_EQUALS ... TK_DOT][TK_OPER] = TK_OPER,

  /* Disambiguation Operators Termination */
  [TK_LANGLE ... TK_DOT][TK_NONE] = TK_OPER,

  /* Literal Propagation */
  [TK_INT][TK_INT]   = TK_INT,
  [TK_INT][TK_UNDER] = TK_INT,
  [TK_INT][TK_IDEN]  = TK_INT,
  [TK_INT][TK_TYPE]  = TK_INT,

  /* Operator Formulation */
  [TK_DASH][TK_RANGLE]   = TK_ARROW,
  [TK_EQUALS][TK_RANGLE] = TK_IMPURE,
  [TK_DOT][TK_DOT]       = TK_RANGE,
  [TK_RANGE][TK_DOT]     = TK_VARARGS,
  [TK_RANGE][TK_LANGLE]  = TK_RANGE_LT,

  /* Indentation */
  [TK_NEWL][TK_NONE] = TK_NEWL,
  [TK_NEWL][TK_NEWL] = TK_NEWL,

  /* Comments */
  [TK_HASH][TK_LBRACK] = TK_LCMT,
  [TK_RBRACK][TK_HASH] = TK_RCMT
};

void
unit_lex(struct unit *restrict unit) {
  assert(unit->tkn_run);

  struct tkn_run *restrict tkn_run = unit->tkn_run;

  size_t i;
  struct tkn tkn = {0};
  tkn.slice.right = -1;

  for (i = 0; i < unit->src_len; i++) {
    enum tkn_kind ch_class = tk_classify[unit->src[i]];

    enum tkn_kind new_kind = tk_transition[tkn.kind][ch_class];
    enum tkn_kind fix_kind = new_kind ? new_kind : tk_transition[TK_NONE][ch_class];

    if (tkn.kind && (!new_kind)) {
      tkn.slice.left = tkn.slice.right + 1;
      tkn.slice.right = i - 1;
      tkn_run = tkn_run_push(unit, tkn_run, tkn);
    }

    tkn.kind = fix_kind;
    tkn.slice.right += (tkn.kind == TK_NONE) ? 1 : 0;
  }
  
  tkn.slice.left = tkn.slice.right + 1;
  tkn.slice.right = i - 1;
  tkn_run_push(unit, tkn_run, tkn);
}

void
unit_lex_print(struct unit *unit) {
  struct tkn_run *tkn_run = unit->tkn_run;
  int indent = 0;

  while (tkn_run) {
    for (size_t i = 0; i < tkn_run->len; i++) {
      for (size_t j = 0; j < tkn_run->tkns[i].indent; j++)
        putchar(' ');
      printf("%s ", tk_debug[tkn_run->tkns[i].kind]);
      if (tkn_run->tkns[i].kind != TK_NEWL)
        unit_print_slice(unit, tkn_run->tkns[i].slice);
      putchar('\n');
    }
    tkn_run = tkn_run->next;
  }
}
