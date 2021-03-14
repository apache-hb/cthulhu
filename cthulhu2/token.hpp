#pragma once

#include <span>
#include <stddef.h>

namespace cthulhu {
    struct Lexer;

    struct Range {
        Lexer* lexer;
        size_t first;
        size_t last;
    };

    struct Token {
        enum Type { 
            KEY, /// a keyword or operator
            IDENT, /// an identifier
            STRING, /// a string literal
            INT, /// an integer literal
            CHAR, /// a character literal
            END /// the end of the file
        };

        Token(Range range);

        std::span<char32_t> text() const;

        const Range range;
        const Type type;
    };

    struct Error {
        enum Type { 
            STRING_EOF, /// unexpected EOF while lexing a string literal
            CHARACTER_EOF, /// unexpected EOF while lexing a character literal
            INVALID_SYMBOL, /// invalid character found in stream
            INTEGER_OVERFLOW, /// a integer literal was too large
            CHARACTER_OVERFLOW, /// a character literal was too large
            STRING_ENCODING, /// a string literal contained an invalid escape sequence
            CHARACTER_ENCODING, /// a character literal contained an invalid escape sequence
        };

        Error(Range range, Type type);

        const Range range;
        const Type type;
    };
}
