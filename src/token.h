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
        INVALID = 4,

        STRING = 5,
        FLOAT = 6,
        INT = 7,
        CHAR = 8
    } type;

    streampos_t pos;

    union {
        keyword key;
        char* ident;
        char* error;
        uint64_t _int;
        char _char;
        double num;
        char* str;
    } data;
} token_t;

char* token_to_string(token_t tok);

#endif // TOKEN_H
