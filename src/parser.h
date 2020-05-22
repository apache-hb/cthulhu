#ifndef PARSER_H
#define PARSER_H

extern "C" {
#include "lexer.h"
}

#include "ast.h"

typedef struct {
    lexer_t source;
    token_t tok;
} parser_t;

parser_t parser_new(lexer_t source);

ast_t produce_ast(parser_t* self);

#endif // PARSER_H
