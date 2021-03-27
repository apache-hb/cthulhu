#pragma once

#include "util.h"

struct range {
    struct lexer* lexer;
    size_t offset;
    size_t length;
};

struct location {
    size_t line;
    size_t column;
};

struct number {
    size_t digit;
    ident suffix;
};

enum struct Key : int {
    INVALID = 0
};

struct token {
    enum type {
        IDENT, // identifier
        KEY, // keyword
        STRING, // string literal
        CHAR, // char literal
        INT, // integer literal
        END, // end of file

        MONOSTATE, // this token doesnt exist

        STRING_EOF, // string wasnt terminated
        STRING_LINE, // newline found in string
        INVALID_ESCAPE, // invalid escaped character in string
        LEADING_ZERO, // an integer literal started with a 0
        INT_OVERFLOW, // integer literal was too large
        UNRECOGNIZED_CHAR, // unrecognized character in stream
        CHAR_OVERFLOW, // character literal was too large
    };

    union data {
        ident ident; // type::IDENT
        ident string; // type::STRING
        Key key; // type::KEY
        ident letters; // type::CHAR
        number digit; // type::INT
    };

    token(range range, type type, data data = {})
        : range(range)
        , type(type)
        , data(data)
    { }

    range range;
    type type;
    data data;
};
