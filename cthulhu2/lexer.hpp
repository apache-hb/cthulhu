#pragma once

#include "token.hpp"

namespace cthulhu {
    struct Lexer {
        Lexer(std::span<char32_t> text);

        std::span<char32_t> slice(size_t offset, size_t length) const;
    private:
        char32_t next();
        char32_t peek();
        char32_t skip();
        bool eat(char32_t c);

        enum Flags : int {
            CORE = (1 << 0), /// lexing core language
            ASM = (1 << 1) /// lexing inline assembly
        };

        // current lexing flags
        Flags flags = CORE;
        // current template depth
        int depth = 0;
        // the start of the current token
        size_t start = 0;
        // the current offset into the file stream
        size_t offset = 0;
        // the source text being lexed
        std::span<char32_t> text;
    };
}
