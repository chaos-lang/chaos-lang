/* lex.c
   author: Simon Lovell Bart (microchips-n-dip)
   description: A new lexer that follows the notions outlined by Sean Barrett
                in his article (http://nothings.org/computer/lexing.html) more
                closely to improve performance.
   date: 2019-06-12
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alias.h"
#include "xxhash.h"
#include "hash_table.h"
#include "lex.h"
#include "ast.h"

/* Initialize a tokenrun. */

void
_init_tokenrun(tokenrun *run) {
  run->next = NULL;
  run->limit = run->tokens + TOKENRUN_SIZE;
}

/* Return the next tokenrun, and allocate it if necessary. */

tokenrun *
next_tokenrun(tokenrun *run) {
  if (run->next == NULL) {
    run->next = XNEW(tokenrun);
    run->next->prev = run;
    _init_tokenrun(run->next);
  }
  return run->next;
}

/* Keyword/reserved word tables. */

#define NODE_KIND_ID  0x0
#define NODE_KIND_RID 0x1

hash_func calc_hash = xxhash32;

static struct table keywords[1];

struct resword {
  const char *key;
  unsigned int len;
  enum rid_code rid;
};

static const struct resword reswords[] = {
  {"Int"   , 3, RID_INT   },
  {"Uint"  , 4, RID_UINT  },
  {"Float" , 5, RID_FLOAT },
  {"Double", 6, RID_DOUBLE},
  {"if"    , 2, RID_IF    },
  {"while" , 5, RID_WHILE },
  {"for"   , 3, RID_FOR   },
};

static const unsigned int num_reswords = 7;

/* Initialize the keyword table. */

void
keywords_init(void) {
  unsigned int i;
  struct node *node;
  create_table(keywords, 8);
  for (i = 0; i < num_reswords; i++) {
    node = lookup(keywords, reswords[i].key, reswords[i].len, INSERT);
    node->kind = NODE_KIND_RID;
    HT_RID(node) = reswords[i].rid;
  }
}

/* Lookup and classify an identifier. */

static inline void
lookup_classify_name(struct token *token) {
  struct node *node;
  node = lookup(keywords, token->str, token->len, INSERT);
  if (node->kind == NODE_KIND_RID) {
    token->type = TOKEN_KEYWORD;
    token->val.rid = HT_RID(node);
  }
  else
    token->val.ident = HT_IDENT(node);
}

/* Include the lexer tables. */

#include "lex_tables.h"

void lex_table_info(void) {
  printf("Equivalence class table size: %ld\n", sizeof(equivalence_class));
  printf("Lexer table size: %ld\n", sizeof(transitions));
  printf("Increment tables size: %ld\n",
         sizeof(stateful_char_increments) + sizeof(stateful_slice_increments));
}

/* Lex an entire unit. */

void
lex_unit(struct unit *unit) {
  enum lexer_state state;
  token *result;
  char c;
  int eq_class;
  while (1) {
    /* Prepare a new token from the tokenrun. */
    result = unit->cur_token++;
    if (unit->cur_token == unit->cur_run->limit) {
      unit->cur_run = next_tokenrun(unit->cur_run);
      unit->cur_token = unit->cur_run->tokens;
    }
    /* Check whether we've reached EOF. */
    if (unlikely(unit->cur == unit->rlimit)) {
      result->type = TOKEN_EOF;
      goto _exit;
    }
    state = LS_START;
    result->len = 0;
    do {
      c = *unit->cur;
      /* Determine the next state to go to. */
      eq_class = equivalence_class[(unsigned char) c];
      state = transitions[state][eq_class];
      result->len += stateful_slice_increments[state];
      /* In the case of increments, we need to reset whenever we've got a
         newline otherwise we'll count empty lines. */
      result->len = (state == LS_NEWLINE && eq_class == EQCLASS_NEWLINE) ?
                    0 : result->len;
      unit->cur += stateful_char_increments[state];
    } while (state < LS_NONTERMINALS_END);
    result->type = state - LS_NONTERMINALS_END + 1;
    result->str = unit->cur - result->len;
    /* Look up any identifiers in the keyword table to determine if they're a
       reserved word. */
    if (result->type == TOKEN_NAME)
      lookup_classify_name(result);
  }
_exit:
  /* We're done, do nothing. */
  do {} while (0);
}

/* Destroy a file reader unit. */

void
destroy_unit(struct unit *unit) {
  /* Just remove the tokenruns for now. */
  tokenrun *run;
  while (unit->cur_run->prev) {
    run = unit->cur_run;
    unit->cur_run = run->prev;
    XDELETE(run);
  }
}
