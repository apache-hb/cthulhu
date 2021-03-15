#pragma once

#include <stddef.h>
#include <string>

struct Lexer;

struct Range {
    Lexer* lexer;
    size_t offset;
    size_t length;
};

enum struct Key {
    INVALID
};

struct Int {
    size_t number;
    std::string* suffix;
};

struct Token {
    enum Type {
        IDENT,
        KEY,
        STRING,
        CHAR,
        INT,
        END
    };

    union Data {
        std::string* ident;
        Key key;
        std::string* string;
        std::string* letter;
        Int number;
    };

    Token(Range range)
        : range(range)
        , type(END)
    { }

    Token(Range range, Type type, Data data)
        : range(range)
        , type(type)
        , data(data)
    { }

    // the internal representation of this token
    std::string repr();

    // the original text of this token
    std::string text();

    Range range;
    Type type;
    Data data;
};
