#ifndef CTU_H
#define CTU_H

#include "thirdparty/vec/vec.h"

#include <stdlib.h>
#include <stdint.h>

typedef void* ctu_file;

typedef int(*ctu_next)(ctu_file);
typedef int(*ctu_peek)(ctu_file);
typedef void(*ctu_seek)(ctu_file, size_t);

typedef struct {
    ctu_next next;
    ctu_peek peek;
    ctu_seek seek;

    ctu_file handle;
} ctu_input;

typedef enum {
    ctu_tt_str,
    ctu_tt_ident,
    ctu_tt_char,
    ctu_tt_int,
    ctu_tt_num,
    ctu_tt_key,
    ctu_tt_eof
} ctu_token_type;

typedef enum {

#define KEYWORD(id, str) id,
#define OPERATOR(id, str) id,
#define ASM_KEYWORD(id, str) id,

#include "keywords.inc"

} ctu_keyword;

typedef struct {
    ctu_token_type type;

    union {
        char* str;
        char* ident;
        char ch;
        int64_t integer;
        double number;
        ctu_keyword key;
    };
} ctu_token;

void ctu_free_token(ctu_token tok);

typedef struct {
    ctu_input input;

    int64_t line;
    int64_t col;
    int64_t pos;

    ctu_token tok;
} ctu_lexer;

ctu_lexer ctu_new_lexer(ctu_input input);
void ctu_free_lexer(ctu_lexer* lexer);

ctu_token ctu_lexer_next(ctu_lexer* lexer);
ctu_token ctu_lexer_peek(ctu_lexer* lexer);

void ctu_print_token(ctu_token tok);

typedef struct {
    ctu_lexer lex;
} ctu_parser;

#endif /* CTU_H */