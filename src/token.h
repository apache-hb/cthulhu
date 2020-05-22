#ifndef TOKEN_H
#define TOKEN_H

#include "keywords.h"
#include "stream.h"

typedef struct {
    enum {
        KEYWORD = 0,
        IDENT = 1,

        // error token
        ERROR = 2,

        // end of file token
        END = 3,

        // invalid token, used by the parser for optional lookahead
        INVALID = 4
    } type;

    streampos_t pos;

    union {
        keyword key;
        char* ident;
        char* error;
    } data;
} token_t;

char* token_to_string(token_t tok);

#endif // TOKEN_H
