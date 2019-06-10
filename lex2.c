enum lexer_state {
  /* Terminal states. */
  LS_STD_TERMINAL,
  LS_TERMINALS,    /* End of terminal state enumeration. */
  LS_WHITESPACE,   /* Whitespace, to be skipped. */
  LS_START,
  /* Normal textual lexing. */
  LS_IDENT,
  LS_NUMBER,
  LS_STRING,
  LS_CHAR,
  /* Operator lexing states. */
  /* Single char operators. */
  LS_SEEN_EQUALS, /* "=" */
  LS_SEEN_GT,     /* ">" */
  LS_SEEN_LT,     /* "<" */
  LS_SEEN_DOT,    /* "." */
  LS_SEEN_COLON,  /* ":" */
  LS_SEEN_PLUS,   /* "+" */
  LS_SEEN_MINUS,  /* "-" */
  LS_SEEN_MUL,    /* "*" */
  LS_SEEN_DIV,    /* "/" */
  /* Double char operators. */
  LS_SEEN_LSHIFT, /* "<<" */
  LS_SEEN_RSHIFT, /* ">>" */
  LS_SEEN_INCR,   /* "++" */
  LS_SEEN_DECR,   /* "--" */
  LS_SEEN_ACCADD, /* "+=" */
  LS_SEEN_ACCSUB, /* "-=" */
  LS_SEEN_ACCMUL, /* "*=" */
  LS_SEEN_ACCDIV, /* "/=" */
  LS_SEEN_EQEQ,   /* "==" */
  LS_SEEN_GTEQ,   /* ">=" */
  LS_SEEN_LTEQ,   /* "<=" */
  LS_SEEN_RARROW, /* "->" */
  LS_SEEN_IMPURE, /* "=>" */
  LS_SEEN_ELLIPSIS_1, /* ".." */
  /* Triple char operators. */
  LS_SEEN_ELLIPSIS, /* "..." */
  LS_OPERATOR,    /* Generic operator. */
  LS_SEEN_TICK,   /* "'" */
  /* Last lexer state. */
  LS_LENGTH
};

struct lexer_transition {
  enum lexer_state state : 8;
  enum token_type type   : 8;
};

static const struct lexer_transition
transitions[LS_LENGTH][EQCLASS_LENGTH] = {
  /* Initial transitions. */
  /* Start of ident. */
  [LS_START][EQCLASS_IDENT]         = {LS_IDENT, TOKEN_NAME},
  /* Start of number. */
  [LS_START][EQCLASS_NUMBER]        = {LS_NUMBER, TOKEN_NUMBER},
  /* Elementary operator encounters. */
  [LS_START][EQCLASS_PLUS]          = {LS_SEEN_PLUS, TOKEN_PLUS},
  [LS_START][EQCLASS_MINUS]         = {LS_SEEN_MINUS, TOKEN_MINUS},
  [LS_START][EQCLASS_MUL]           = {LS_SEEN_MUL, TOKEN_MUL},
  [LS_START][EQCLASS_DIV]           = {LS_SEEN_DIV, TOKEN_DIV},
  /* Encountered '='. */
  [LS_START][EQCLASS_EQUALS]        = {LS_SEEN_EQUALS, TOKEN_EQUALS},
  /* Elementary comparison encounters. */
  [LS_START][EQCLASS_GT]            = {LS_SEEN_GT, TOKEN_GT},
  [LS_START][EQCLASS_LT]            = {LS_SEEN_LT, TOKEN_LT},
  /* Misc. */
  [LS_START][EQCLASS_DOT]           = {LS_SEEN_DOT, TOKEN_DOT},
  [LS_START][EQCLASS_COLON]         = {LS_SEEN_COLON, TOKEN_COLON},
  [LS_START][EQCLASS_TICK]          = {LS_SEEN_TICK, TOKEN_TICK},
  /* Normal textual transitions. */
  [LS_IDENT][EQCLASS_IDENT]         = {LS_IDENT, TOKEN_NAME},
  [LS_IDENT][EQCLASS_NUMBER]        = {LS_IDENT, TOKEN_NAME},
  [LS_NUMBER][EQCLASS_NUMBER]       = {LS_NUMBER, TOKEN_NUMBER},
  /* TODO: Strings and chars. */
  /* Operator transitions. */
  /* Encountered '+' then '+'. */
  [LS_SEEN_PLUS][EQCLASS_PLUS]      = {LS_SEEN_INCR, TOKEN_INCR},
  /* Encountered '+' then '='. */
  [LS_SEEN_PLUS][EQCLASS_EQUALS]    = {LS_SEEN_ACCADD, TOKEN_ACCADD},
  /* Encountered '+' then any of {'-', '*', '/'}. */
  [LS_SEEN_PLUS][EQCLASS_MINUS ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '+' then any of {'>', '<', ':'}. */
  [LS_SEEN_PLUS][EQCLASS_GT ... EQCLASS_COLON]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '-' then '-'. */
  [LS_SEEN_MINUS][EQCLASS_MINUS]    = {LS_SEEN_DECR, TOKEN_DECR},
  /* Encountered '-' then '='. */
  [LS_SEEN_MINUS][EQCLASS_EQUALS]   = {LS_SEEN_ACCSUB, TOKEN_ACCSUB},
  /* Encountered '-' then any of {'>, '<', ':', '+'}. */
  [LS_SEEN_MINUS][EQCLASS_GT ... EQCLASS_PLUS]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '-' then any of {'*', '/'}. */
  [LS_SEEN_MINUS][EQCLASS_MUL ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '*' then '='. */
  [LS_SEEN_MUL][EQCLASS_EQUALS]     = {LS_SEEN_ACCMUL, TOKEN_ACCMUL},
  /* Encountered '*' then any of {'>', '<', ':', '+', '-', '*', '/'}. */
  [LS_SEEN_MUL][EQCLASS_GT ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '/' then '='. */
  [LS_SEEN_DIV][EQCLASS_EQUALS]     = {LS_SEEN_ACCDIV, TOKEN_ACCDIV},
  /* Encountered '/' then any of {'>', '<', ':', '+', '-', '*', '/'}. */
  [LS_SEEN_DIV][EQCLASS_GT ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '=' then '='. */
  [LS_SEEN_EQUALS][EQCLASS_EQUALS]  = {LS_SEEN_EQEQ, TOKEN_EQEQ},
  /* Encountered '=' then '>'. */
  [LS_SEEN_EQUALS][EQCLASS_GT]      = {LS_SEEN_IMPURE, TOKEN_IMPURE},
  /* Encountered '=' then any of {'<', ':', '+', '-', '*', '/'}. */
  [LS_SEEN_EQUALS][EQCLASS_LT ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '>' then '='. */
  [LS_SEEN_GT][EQCLASS_EQUALS]      = {LS_SEEN_GTEQ, TOKEN_GTEQ},
  /* Encountered '>' then '>'. */
  [LS_SEEN_GT][EQCLASS_GT]          = {LS_SEEN_RSHIFT, TOKEN_RSHIFT},
  /* Encountered '>' then any of {'<', ':', '+', '-', '*', '/'}. */
  [LS_SEEN_GT][EQCLASS_LT ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '<' then '='. */
  [LS_SEEN_LT][EQCLASS_EQUALS]      = {LS_SEEN_LTEQ, TOKEN_LTEQ},
  /* Encountered '<' then '<'. */
  [LS_SEEN_LT][EQCLASS_LT]          = {LS_SEEN_LSHIFT, TOKEN_LSHIFT},
  /* Encountered '<' then '>'. */
  [LS_SEEN_LT][EQCLASS_GT]          = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '<' then any of {':', '+', '-', '*', '/'}. */
  [LS_SEEN_LT][EQCLASS_COLON ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR}
  /* Encountered ':' then anything. */
  [LS_SEEN_COLON][EQCLASS_COLON ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR}
  /* Encountered '.' then '.'. */
  [LS_SEEN_DOT][EQCLASS_DOT]        = {LS_SEEN_ELLIPSIS_1, TOKEN_OPERATOR},
  /* Encountered '.' then any of {'=', '>', '<'}. */
  [LS_SEEN_DOT][EQCLASS_EQUALS ... EQCLASS_LT]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered '.' then any of {':', '+', '-', '*', '/'}. */
  [LS_SEEN_DOT][EQCLASS_COLON ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* All of the double char operators can just go to LS_OPERATOR if they
     encounter anything else. */
  [LS_SEEN_LSHIFT ... LS_SEEN_IMPURE][EQCLASS_EQUALS ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Encountered ".." then '.'. */
  [LS_SEEN_ELLIPSIS_1][EQCLASS_DOT] = {LS_SEEN_ELLIPSIS, TOKEN_ELLIPSIS},
  /* Encountered ".." then any of {'=', '>', '<'}. */
  [LS_SEEN_ELLIPSIS_1][EQCLASS_EQUALS ... EQCLASS_LT]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* There are no 4-char operators so just deal with this, yeah? */
  [LS_SEEN_ELLIPSIS][EQCLASS_EQUALS ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* Finally, for things that are already unrecognized operators. */
  [LS_OPERATOR][EQCLASS_EQUALS ... EQCLASS_DIV]
                                    = {LS_OPERATOR, TOKEN_OPERATOR},
  /* */
};

/* Stateful increments prevent incrementing on terminal states and
   whitespace. */

static const char
stateful_increments[LS_LENGTH] = {
  [LS_START ... LS_LENGTH] = 1
};

void
lex_unit(struct unit *unit) {
  enum lexer_state state;
  token *result;
  char c;
  int eq_class, disambiguation_hash;
  while (1) {
    /* Prepare a new token from the tokenrun. */
    result = unit->cur_token++;
    if (unit->cur_token == unit->cur_run->limit) {
      unit->cur_run = next_tokenrun(unit->cur_run);
      unit->cur_token = unit->cur_run->tokens;
    }
    /* Check whether we've reached EOF. */
    if (unlikely(i == unit->len)) {
      result->type = TOKEN_EOF;
      goto _exit;
    }
    state = LS_START;
    do {
      c = *unit->cur++;
      /* Determine the next state to go to. */
      eq_class = eqivalences[c];
      state = transitions[state][eq_class].state;
      /* Force use of conditional moves on supported arches to speed things up.
         With regards to Linus's rant on the topic of `cmov`: although in older
         CPUs with more aggressive OoO he is correct in his assessment that
         `cmov` may perform just as well if not worse than predicted branches,
         modern architectures appear to no longer have that issue
         (see https://github.com/xiadz/cmov), and since this is a lexer,
         branching in this SM loop will be unpredictable so lots of recovery
         would happen. */
#if defined _ARCH_X86_
      asm volatile (
        "\tcmp %[state], %[terminals]\n"
        "\tcmovl %[type], %[type]\n"
        "\tcmovg %[new_type], %[type]\n"
        : [type] "+r" (result->type)
        : [state] "r" (state),
          [terminals] "N" (LS_TERMINALS),
          [new_type] "r" (new_type)
      );
#elif defined _ARCH_ARM_
      asm volatile (
        "\tcmp %[state], %[terminals]\n"
        "\tmovlt %[type], %[type]\n"
        "\tmovgt %[type], %[new_type]\n"
        : [type] "+r" (result->type)
        : [state] "r" (state),
          [terminals] "N" (LS_TERMINALS),
          [new_type] "r" (new_type)
      );
#else
      result->type = transitions[state][eq_class].type;
#endif
      result->len += stateful_increments[state];
    } while (state > LS_TERMINALS);
  }
_exit:
  /* We're done, do nothing. */
  do {} while (0);
}
