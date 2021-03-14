#pragma once

#include <span>

namespace cthulhu {
    struct Lexer;

    struct Token {
        enum Type { KEY, IDENT, STRING, INT, CHAR, END };

        Token(Lexer* lexer, size_t first, size_t last);

        std::span<char32_t> text() const;
        
    private:
        Lexer* lexer;
        size_t first;
        size_t last;

        Type type;
    };
}
