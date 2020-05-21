#ifndef TOKEN_H
#define TOKEN_H

#include "keywords.h"
#include "stream.h"

typedef struct {
    enum {
        KEYWORD,
        IDENT,

        // error token
        ERROR,

        // end of file token
        END,

        // invalid token, used by the parser for optional lookahead
        INVALID
    } type;

    streampos_t pos;

    union {
        keyword key;
        char* ident;
        char* error;
    } data;
} token_t;

#endif // TOKEN_H
