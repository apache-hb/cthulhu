#pragma once

#include <stddef.h>
#include <string>

struct Lexer;

struct Range {
    Lexer* lexer;
    size_t offset;
    size_t length;
};

struct Location {
    size_t line;
    size_t column;
};

enum struct Key {
    INVALID,
#define KEY(id, str) id,
#define ASM(id, str) id,
#define OP(id, str) id,
#include "keys.inc"
};

struct Int {
    size_t number;
    const std::string* suffix;
};

struct Token {
    enum Type {
        IDENT, // identifier
        KEY, // keyword
        STRING, // string literal
        CHAR, // char literal
        INT, // integer literal
        END, // end of file

        ERROR,
        ERROR_STRING_EOF, // string wasnt terminated
        ERROR_STRING_LINE, // newline found in string
        ERROR_INVALID_ESCAPE, // invalid escaped character in string
        ERROR_LEADING_ZERO, // an integer literal started with a 0
        ERROR_INT_OVERFLOW, // integer literal was too large
    };

    union Data {
        const std::string* ident;
        Key key;
        const std::string* string;
        std::string* letter;
        Int number;
    };

    Token(Range range)
        : range(range)
        , type(END)
    { }

    Token(Range range, Type type, Data data = {})
        : range(range)
        , type(type)
        , data(data)
    { }

    // the internal representation of this token
    std::string repr();

    // the original text of this token
    std::string text();

    // underline the text and handle multiple lines
    std::string pretty(bool underline = true, bool colour = true, const std::string& message = "");

    // is this token an error token
    bool error() const;

    Range range;
    Type type;
    Data data;
};
