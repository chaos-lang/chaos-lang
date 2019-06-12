enum equivalence_classes {
  EQCLASS_WHITESPACE,
  EQCLASS_NEWLINE,
  EQCLASS_LETTER,
  EQCLASS_NUMBER,
  EQCLASS_EQUALS,
  EQCLASS_COLON,
  EQCLASS_MINUS,
  EQCLASS_DOT,
  EQCLASS_GT,
  EQCLASS_OPERATOR
};

static const char
equivalence_class[256] = {
  [' ']         = EQCLASS_WHITESPACE,
  ['\t']        = EQCLASS_WHITESPACE,
  ['\n']        = EQCLASS_NEWLINE,
  ['A' ... 'Z'] = EQCLASS_LETTER,
  ['a' ... 'z'] = EQCLASS_LETTER,
  ['0' ... '9'] = EQCLASS_NUMBER,
  ['=']         = EQCLASS_EQUALS,
  [':']         = EQCLASS_COLON,
  ['+']         = EQCLASS_OPERATOR,
  ['-']         = EQCLASS_MINUS,
  ['*']         = EQCLASS_OPERATOR,
  ['/']         = EQCLASS_OPERATOR,
  ['.']         = EQCLASS_DOT,
  ['>']         = EQCLASS_GT,
  ['<']         = EQCLASS_OPERATOR,
};

enum lexer_state {
  /* Terminal states. */
  LS_NEWLINE_TERMINAL  = -10,
  LS_IDENT_TERMINAL    = -9,
  LS_NUMBER_TERMINAL   = -8,
  LS_EQUALS_TERMINAL   = -7,
  LS_COLON_TERMINAL    = -6,
  LS_RARROW_TERMINAL   = -5,
  LS_IMPURE_TERMINAL   = -4,
  LS_ELLIPSIS_TERMINAL = -3,
  LS_OPERATOR_TERMINAL = -2,
  LS_TERMINALS         = -1,
  LS_START             =  0,
  LS_WHITESPACE,   /* Whitespace, to be skipped. */
  /* Normal textual lexing. */
  LS_IDENT,
  LS_NUMBER,
  LS_STRING,
  LS_CHAR,
  /* Operator lexing states. */
  /* Single char operators. */
  LS_SEEN_EQUALS,   /* "=" */
  LS_SEEN_COLON,    /* ":" */
  LS_SEEN_MINUS,    /* "-" */
  LS_SEEN_RARROW,   /* "->" */
  LS_SEEN_IMPURE,   /* "=>" */
  LS_SEEN_DOT,      /* "." */
  LS_SEEN_DOT_2,    /* ".." */
  /* Triple char operators. */
  LS_SEEN_ELLIPSIS, /* "..." */
  LS_OPERATOR,      /* Generic operator. */
  LS_SEEN_TICK      /* "'" */
};

/* Total number of states. */

static const char LS_LENGTH = LS_SEEN_TICK;

/* State machine transitions table. */

static const signed char
transitions[LS_LENGTH][EQCLASS_LENGTH] = {
  /* Initial transitions. */
  [LS_START][EQCLASS_WHITESPACE]       = LS_WHITESPACE,
  [LS_START][EQCLASS_NEWLINE]          = LS_NEWLINE,
  [LS_START][EQCLASS_IDENT]            = LS_IDENT,
  [LS_START][EQCLASS_NUMBER]           = LS_NUMBER,
  [LS_START][EQCLASS_EQUALS]           = LS_SEEN_EQUALS,
  [LS_START][EQCLASS_COLON]            = LS_SEEN_COLON,
  [LS_START][EQCLASS_MINUS]            = LS_SEEN_MINUS,
  [LS_START][EQCLASS_DOT]              = LS_SEEN_DOT,
  [LS_START][EQCLASS_GT]               = LS_OPERATOR,
  [LS_START][EQCLASS_OPERATOR]         = LS_OPERATOR,
  [LS_START][EQCLASS_TICK]             = LS_SEEN_TICK,
  /* Whitespace transitions. */
  [LS_WHITESPACE][EQCLASS_WHITESPACE]  = LS_WHITESPACE,
  [LS_WHITESPACE][EQCLASS_NEWLINE]     = LS_WHITESPACE,
  [LS_WHITESPACE][EQCLASS_IDENT]       = LS_START,
  [LS_WHITESPACE][EQCLASS_NUMBER]      = LS_START,
  [LS_WHITESPACE][EQCLASS_EQUALS]      = LS_START,
  [LS_WHITESPACE][EQCLASS_COLON]       = LS_START,
  [LS_WHITESPACE][EQCLASS_MINUS]       = LS_START,
  [LS_WHITESPACE][EQCLASS_DOT]         = LS_START,
  [LS_WHITESPACE][EQCLASS_GT]          = LS_START,
  [LS_WHITESPACE][EQCLASS_OPERATOR]    = LS_START,
  [LS_NEWLINE][EQCLASS_WHITESPACE]     = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_NEWLINE]        = LS_NEWLINE,
  [LS_NEWLINE][EQCLASS_IDENT]          = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_NUMBER]         = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_EQUALS]         = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_COLON]          = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_MINUS]          = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_DOT]            = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_GT]             = LS_NEWLINE_TERMINAL,
  [LS_NEWLINE][EQCLASS_OPERATOR]       = LS_NEWLINE_TERMINAL,
  /* Normal textual transitions. */
  [LS_IDENT][EQCLASS_WHITESPACE]       = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_NEWLINE]          = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_IDENT]            = LS_IDENT,
  [LS_IDENT][EQCLASS_NUMBER]           = LS_IDENT,
  [LS_IDENT][EQCLASS_EQUALS]           = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_COLON]            = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_MINUS]            = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_DOT]              = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_GT]               = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_OPERATOR]         = LS_IDENT_TERMINAL,
  [LS_IDENT][EQCLASS_TICK]             = LS_IDENT_TERMINAL,
  [LS_NUMBER][EQCLASS_WHITESPACE]      = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_NEWLINE]         = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_IDENT]           = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_NUMBER]          = LS_NUMBER,
  [LS_NUMBER][EQCLASS_EQUALS]          = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_COLON]           = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_MINUS]           = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_DOT]             = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_GT]              = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_OPERATOR]        = LS_NUMBER_TERMINAL,
  [LS_NUMBER][EQCLASS_TICK]            = LS_NUMBER_TERMINAL,
  /* TODO: Strings and chars. */
  /* Operator transitions. */
  [LS_SEEN_EQUALS][EQCLASS_WHITESPACE] = LS_EQUALS_TERMINAL,
  [LS_SEEN_EQUALS][EQCLASS_NEWLINE]    = LS_EQUALS_TERMINAL,
  [LS_SEEN_EQUALS][EQCLASS_IDENT]      = LS_EEUALS_TERMINAL,
  [LS_SEEN_EQUALS][EQCLASS_NUMBER]     = LS_EQUALS_TERMINAL,
  [LS_SEEN_EQUALS][EQCLASS_EQUALS]     = LS_OPERATOR,
  [LS_SEEN_EQUALS][EQCLASS_COLON]      = LS_OPERATOR,
  [LS_SEEN_EQUALS][EQCLASS_MINUS]      = LS_OPERATOR,
  [LS_SEEN_EQUALS][EQCLASS_DOT]        = LS_OPERATOR,
  [LS_SEEN_EQUALS][EQCLASS_GT]         = LS_SEEN_IMPURE,
  [LS_SEEN_EQUALS][EQCLASS_OPERATOR]   = LS_OPERATOR,
  [LS_SEEN_COLON][EQCLASS_WHITESPACE]  = LS_COLON_TERMINAL,
  [LS_SEEN_COLON][EQCLASS_NEWLINE]     = LS_COLON_TERMINAL,
  [LS_SEEN_COLON][EQCLASS_IDENT]       = LS_COLON_TERMINAL,
  [LS_SEEN_COLON][EQCLASS_NUMBER]      = LS_COLON_TERMINAL,
  [LS_SEEN_COLON][EQCLASS_EQUALS]      = LS_OPERATOR,
  [LS_SEEN_COLON][EQCLASS_COLON]       = LS_OPERATOR,
  [LS_SEEN_COLON][EQCLASS_MINUS]       = LS_OPERATOR,
  [LS_SEEN_COLON][EQCLASS_DOT]         = LS_OPERATOR,
  [LS_SEEN_COLON][EQCLASS_GT]          = LS_OPERATOR,
  [LS_SEEN_COLON][EQCLASS_OPERATOR]    = LS_OPERATOR,
  [LS_SEEN_MINUS][EQCLASS_WHITESPACE]  = LS_OPERATOR_TERMINAL,
  [LS_SEEN_MINUS][EQCLASS_NEWLINE]     = LS_OPERATOR_TERMINAL,
  [LS_SEEN_MINUS][EQCLASS_IDENT]       = LS_OPERATOR_TERMINAL,
  [LS_SEEN_MINUS][EQCLASS_NUMBER]      = LS_OPERATOR_TERMINAL,
  [LS_SEEN_MINUS][EQCLASS_EQUALS]      = LS_OPERATOR,
  [LS_SEEN_MINUS][EQCLASS_COLON]       = LS_OPERATOR,
  [LS_SEEN_MINUS][EQCLASS_MINUS]       = LS_OPERATOR,
  [LS_SEEN_MINUS][EQCLASS_DOT]         = LS_OPERATOR,
  [LS_SEEN_MINUS][EQCLASS_GT]          = LS_SEEN_RARROW,
  [LS_SEEN_MINUS][EQCLASS_OPERATOR]    = LS_OPERATOR,
  [LS_SEEN_DOT][EQCLASS_WHITESPACE]    = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT][EQCLASS_NEWLINE]       = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT][EQCLASS_IDENT]         = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT][EQCLASS_NUMBER]        = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT][EQCLASS_EQUALS]        = LS_OPERATOR,
  [LS_SEEN_DOT][EQCLASS_COLON]         = LS_OPERATOR,
  [LS_SEEN_DOT][EQCLASS_MINUS]         = LS_OPERATOR,
  [LS_SEEN_DOT][EQCLASS_DOT]           = LS_SEEN_DOT_2,
  [LS_SEEN_DOT][EQCLASS_GT]            = LS_OPERATOR,
  [LS_SEEN_DOT][EQCLASS_OPERATOR]      = LS_OPERATOR,
  [LS_SEEN_RARROW][EQCLASS_WHITESPACE] = LS_RARROW_TERMINAL,
  [LS_SEEN_RARROW][EQCLASS_NEWLINE]    = LS_RARROW_TERMINAL,
  [LS_SEEN_RARROW][EQCLASS_IDENT]      = LS_RARROW_TERMINAL,
  [LS_SEEN_RARROW][EQCLASS_NUMBER]     = LS_RARROW_TERMINAL,
  [LS_SEEN_RARROW][EQCLASS_EQUALS]     = LS_OPERATOR,
  [LS_SEEN_RARROW][EQCLASS_COLON]      = LS_OPERATOR,
  [LS_SEEN_RARROW][EQCLASS_MINUS]      = LS_OPERATOR,
  [LS_SEEN_RARROW][EQCLASS_DOT]        = LS_OPERATOR,
  [LS_SEEN_RARROW][EQCLASS_GT]         = LS_OPERATOR,
  [LS_SEEN_RARROW][EQCLASS_OPERATOR]   = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_WHITESPACE] = LS_IMPURE_TERMINAL,
  [LS_SEEN_IMPURE][EQCLASS_NEWLINE]    = LS_IMPURE_TERMINAL,
  [LS_SEEN_IMPURE][EQCLASS_IDENT]      = LS_IMPURE_TERMINAL,
  [LS_SEEN_IMPURE][EQCLASS_NUMBER]     = LS_IMPURE_TERMINAL,
  [LS_SEEN_IMPURE][EQCLASS_EQUALS]     = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_COLON]      = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_MINUS]      = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_DOT]        = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_GT]         = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_OPERATOR]   = LS_OPERATOR,
  [LS_SEEN_IMPURE][EQCLASS_TICK]       = LS_OPERATOR,
  [LS_SEEN_DOT_2][EQCLASS_WHITESPACE]  = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT_2][EQCLASS_NEWLINE]     = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT_2][EQCLASS_IDENT]       = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT_2][EQCLASS_NUMBER]      = LS_OPERATOR_TERMINAL,
  [LS_SEEN_DOT_2][EQCLASS_EQUALS]      = LS_OPERATOR,
  [LS_SEEN_DOT_2][EQCLASS_COLON]       = LS_OPERATOR,
  [LS_SEEN_DOT_2][EQCLASS_MINUS]       = LS_OPERATOR,
  [LS_SEEN_DOT_2][EQCLASS_DOT]         = LS_SEEN_DOT_3,
  [LS_SEEN_DOT_2][EQCLASS_GT]          = LS_OPERATOR,
  [LS_SEEN_DOT_2][EQCLASS_OPERATOR]    = LS_OPERATOR,
  [LS_SEEN_DOT_3][EQCLASS_WHITESPACE]  = LS_ELLIPSIS_TERMINAL,
  [LS_SEEN_DOT_3][EQCLASS_NEWLINE]     = LS_ELLIPSIS_TERMINAL,
  [LS_SEEN_DOT_3][EQCLASS_IDENT]       = LS_ELLIPSIS_TERMINAL,
  [LS_SEEN_DOT_3][EQCLASS_NUMBER]      = LS_ELLIPSIS_TERMINAL,
  [LS_SEEN_DOT_3][EQCLASS_EQUALS]      = LS_OPERATOR,
  [LS_SEEN_DOT_3][EQCLASS_COLON]       = LS_OPERATOR,
  [LS_SEEN_DOT_3][EQCLASS_MINUS]       = LS_OPERATOR,
  [LS_SEEN_DOT_3][EQCLASS_DOT]         = LS_OPERATOR,
  [LS_SEEN_DOT_3][EQCLASS_GT]          = LS_OPERATOR,
  [LS_SEEN_DOT_3][EQCLASS_OPERATOR]    = LS_OPERATOR,
  [LS_OPERATOR][EQCLASS_WHITESPACE]    = LS_OPERATOR_TERMINAL,
  [LS_OPERATOR][EQCLASS_NEWLINE]       = LS_OPERATOR_TERMINAL,
  [LS_OPERATOR][EQCLASS_IDENT]         = LS_OPERATOR_TERMINAL,
  [LS_OPERATOR][EQCLASS_NUMBER]        = LS_OPERATOR_TERMINAL,
  [LS_OPERATOR][EQCLASS_EQUALS]        = LS_OPERATOR,
  [LS_OPERATOR][EQCLASS_COLON]         = LS_OPERATOR,
  [LS_OPERATOR][EQCLASS_MINUS]         = LS_OPERATOR,
  [LS_OPERATOR][EQCLASS_DOT]           = LS_OPERATOR,
  [LS_OPERATOR][EQCLASS_GT]            = LS_OPERATOR,
  [LS_OPERATOR][EQCLASS_OPERATOR]      = LS_OPERATOR,
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
      result->len += stateful_increments[state];
    } while (state > LS_TERMINALS);
    result->type = -state;
  }
_exit:
  /* We're done, do nothing. */
  do {} while (0);
}
