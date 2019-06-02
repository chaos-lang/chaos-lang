#ifndef LEX_H
#define LEX_H

/* String impl. */

typedef struct String String;

struct String {
  const char *base;
  unsigned int len;
};

/* Token impl. */

/* Token type enumeration. */

enum token_type {
  TOKEN_OTHER,         /* An unknown character. */
  TOKEN_NAME,          /* Name/identifier token. */
  TOKEN_KEYWORD,       /* Keyword/reserved word token. */
  TOKEN_NUMBER,        /* A number token. */
  TOKEN_STRING,        /* A string literal token. */
  TOKEN_CHAR,
  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACKET,
  TOKEN_OPEN_BRACE,
  TOKEN_CLOSE_BRACE,
  TOKEN_LANGLE,
  TOKEN_RANGLE,
  TOKEN_LARROW,
  TOKEN_RARROW,
  TOKEN_EQUALS,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MULT,
  TOKEN_DIV,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_COMMENT,
  TOKEN_SEMICOLON,
  TOKEN_EOF,
  LTKN_WHITESPACE      /* This isn't a real token per-se. */
};

typedef struct token token;

struct token {
  enum token_type type;
  unsigned int base_index;
  union {
    int integer;
    String str;
    //enum rid_code rid;
  } val;
};

/* Tokenruns. */

/* Tokenruns are sized statically rather than dynamically so that, at the cost
   of being able to change their size, we can allocate them in one go rather
   than two. */

#define TOKENRUN_SIZE 256

typedef struct tokenrun tokenrun;

struct tokenrun {
  tokenrun *prev, *next;
  token tokens[TOKENRUN_SIZE];
  token *limit;
};

extern void _init_tokenrun(tokenrun *);
extern tokenrun *next_tokenrun(tokenrun *);

/* A lexer unit; handles a single file. */

typedef struct Unit Unit;

struct Unit {
  /* Current position in the buffer. */
  const char *cur;
  /* The buffer itself. */
  const char *buf;
  /* The buffer limit, used for detecting EOF. */
  const char *rlimit;
  /* Flag indicating whether to replace the next newline with an EOL. */
  unsigned int need_eol : 1;
  Unit *prev;
  /* Tokenruns (put in the outer reader struct?) */
  tokenrun base_run;
  tokenrun *cur_run;
  token *cur_token;
};

extern void lex_unit(Unit *);
extern void destroy_unit(Unit *);

#endif
