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

struct {
  size_t cmt_depth; // #[ comment ]#
  char   cmt_hash;  // # comment
} push_state = {0};

/* Helper Functions */

static char tk_oper_classify[768] = {
  ['-' + '>'] = TK_ARROW,
  ['=' + '>'] = TK_IMPURE,
  ['.' + '.'] = TK_RANGE,
  ['.' + '.' + '<'] = TK_RANGE_LT,
  ['.' + '.' + '.'] = TK_VARARGS
};

static char *tk_oper_string[TK_LENGTH] = {
  [TK_ARROW]    = "->",
  [TK_IMPURE]   = "=>",
  [TK_RANGE]    = "..",
  [TK_RANGE_LT] = "..<",
  [TK_VARARGS]  = "..."
};

static inline struct tkn_run *
tkn_run_push(struct unit *unit, struct tkn_run *tkn_run, struct tkn tkn) {
  assert(tkn_run != NULL);
  assert(tkn_run->len <= CHAOS_TKN_RUN_LEN);

  if (unlikely(push_state.cmt_depth || push_state.cmt_hash)) {
    push_state.cmt_depth += (tkn.kind == TK_LCMT) ? 1 : 0;
    push_state.cmt_depth -= (tkn.kind == TK_RCMT) ? (push_state.cmt_depth ? 1 : 0) : 0;
    push_state.cmt_hash = (tkn.kind == TK_NEWL) ? 0 : 1;
    return tkn_run;
  }

  switch (tkn.kind) {
    case TK_NONE: {
      return tkn_run;
    } break;

    case TK_HASH: {
      push_state.cmt_hash = 1;
      return tkn_run;
    } break;
    
    case TK_OPER: {
      size_t hash = 0;
      for (size_t i = tkn.slice.left; i <= tkn.slice.right; i++)
        hash += unit->src[i];
      enum tkn_kind kind = tk_oper_classify[hash];
      tkn.kind = kind && unit_slice_cmp_str(unit, tkn.slice, tk_oper_string[kind]) ? kind : tkn.kind;
    } break;

    case TK_LCMT: {
      push_state.cmt_depth++;
      return tkn_run;
    } break;

    case TK_INT: {
      tkn.val.uint = unit_slice_atoi(unit, tkn.slice);
    } break;
  }

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

static char tk_classify[256] = {
  ['a' ... 'z'] = TK_IDEN,
  ['A' ... 'Z'] = TK_TYPE,
  
  ['+'] = TK_OPER,
  ['-'] = TK_OPER,
  ['*'] = TK_OPER,
  ['/'] = TK_OPER,
  ['<'] = TK_OPER,
  ['>'] = TK_OPER,
  ['$'] = TK_OPER,
  ['#'] = TK_HASH,
  [','] = TK_COMMA,
  ['='] = TK_EQUALS,
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

static char tk_transition[TK_LENGTH][TK_LENGTH] = {
  /* None -> State */
  [TK_NONE][TK_IDEN] = TK_IDEN,
  [TK_NONE][TK_TYPE] = TK_TYPE,

  [TK_NONE][TK_OPER] = TK_OPER,
  [TK_NONE][TK_HASH] = TK_HASH,
  [TK_NONE][TK_COMMA] = TK_COMMA,
  [TK_NONE][TK_EQUALS] = TK_EQUALS,
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

  [TK_OPER][TK_OPER] = TK_OPER,

  /* Comments */
  [TK_HASH][TK_LBRACK] = TK_LCMT,
  [TK_RBRACK][TK_HASH] = TK_RCMT,

  /* Re-Ambiguation Codes */
  [TK_EQUALS][TK_OPER] = TK_OPER
};

void
unit_lex(struct unit *unit) {
  assert(unit->tkn_run);

  struct tkn_run *tkn_run = unit->tkn_run;

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
