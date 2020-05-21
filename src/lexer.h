#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "stream.h"

typedef struct {
    stream_t source;
    char buffer[512];
    int idx;
} lexer_t;

lexer_t lexer_new(stream_t source);

token_t lexer_next(lexer_t* self);

#endif // LEXER_H
