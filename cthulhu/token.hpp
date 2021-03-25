#pragma once

#include "fwd.hpp"

#include <stddef.h>

namespace cthulhu {
    struct Range {
        Range();
        Range(Lexer* lexer);
        Range(Lexer* lexer, size_t offset, size_t line, size_t column, size_t length);

        Range to(const Range& end) const;

        Lexer* lexer;
        size_t offset;
        size_t line;
        size_t column;
        size_t length;
    };

    enum struct Key {
        INVALID,
#define KEY(id, _) id,
#define OP(id, _) id,
#include "keys.inc"
    };

    struct Number {
        Number(size_t num, const str* suf);
        size_t number;
        const str* suffix;
    };

    union TokenData {
        Key key;
        const str* ident;
        const str* string;
        char32_t letter;
        Number digit;
    };

    struct Token {
        enum Type {
            IDENT,
            KEY,
            STRING,
            INT,
            CHAR,
            END,
            INVALID
        };

        Token();
        Token(Type type, TokenData data);
        Token(Range where, Type type, TokenData data);

        bool is(Type other) const;
        bool operator==(const Token& other) const;
        bool operator!=(const Token& other) const;
        bool equals(const Token& other) const;
        operator bool() const { return valid(); }

        Key key() const;
        const str* ident() const;
        const str* string() const;
        Number number() const;
        char32_t letter() const;
        bool valid() const;
    private:
        Range where;
        Type type;
        TokenData data;
    };
}
