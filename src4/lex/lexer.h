#pragma once

#include "token.h"

#include <memory>

namespace ct {

    struct Lexer {
        Token next() {
            int c = read();

            switch (c) {
            case '\0':
                break;
            }
        }

    private:

        Token lex_ident() {

        }

        int read() {
            int temp = ahead;
            ahead = in->next();

            pos.pos++;
            if (temp == '\n') {
                pos.col = 0;
                pos.line++;
            } else {
                pos.col++;
            }

            return temp;
        }

        int peek() const {
            return ahead;
        }

        std::unique_ptr<std::istream> in;
        int ahead;

        SourcePosition pos;
    };
}