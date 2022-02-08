#pragma once

#include "cthulhu/ast/ast.h"

typedef enum {
    KEY_INVALID,

    KEY_VOID,

    KEY_CHAR,
    KEY_SHORT,
    KEY_INT,
    KEY_LONG,

    KEY_SIGNED,
    KEY_UNSIGNED,

    KEY_CONST,
    KEY_VOLATILE,

    KEY_EXTERN,

    KEY_LPAREN,
    KEY_RPAREN,

    KEY_LBRACE,
    KEY_RBRACE,

    KEY_SEMICOLON,

    KEY_STAR,

    KEY_COMMA,

    KEY_DOT,
    KEY_DOT2,
    KEY_DOT3
} c11_keyword_t;

void c11_keyword_init(void);

c11_keyword_t c11_keyword_lookup(const char *name);

typedef enum {
    TOK_KEYWORD,
    TOK_IDENT,
    TOK_STRING,

    TOK_EOF,
    TOK_INVALID /* something went wrong */
} c11_token_type_t;

typedef struct {
    c11_token_type_t type;
    node_t *node;

    union {
        c11_keyword_t keyword; // the keyword
        char *ident; // the encoded ident
        char *string; // the parsed and encoded string
        const char *error; // the error message
    };
} c11_token_t;

typedef struct {
    scan_t *scan;
    where_t where;
    size_t offset;
    char ahead;
} c11_lexer_state_t;

typedef struct {
    c11_lexer_state_t *state; // our current state
    c11_token_t token;
} c11_lexer_t;

void c11_keywords_init(void);

c11_lexer_t *c11_lexer_new(scan_t *scan);

c11_token_t c11_lexer_next(c11_lexer_t *lexer);
c11_token_t c11_lexer_peek(c11_lexer_t *lexer);

bool tok_is_key(c11_token_t tok, c11_keyword_t keyword);
bool tok_is_ident(c11_token_t tok);
bool tok_is_eof(c11_token_t tok);
