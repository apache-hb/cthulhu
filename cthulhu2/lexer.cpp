#include "lexer.h"

namespace {
    bool ident1(char c) {
        return isalpha(c) || c == '_';
    }

    bool ident2(char c) {
        return isalnum(c) || c == '_';
    }
}

Token Lexer::read() {
    char c = skip();
    if (c == 0) {
        return Token(here());
    } else if (ident1(c)) {
        while (ident2(peek()))
            next();

        return Token(here(), Token::IDENT, { .ident = new std::string("text") });
    } else {
        return Token(here());
    }
}

char Lexer::skip() {
    char c = next();
    
    while (isspace(c)) {
        c = next();
    }

    start = offset - 1;

    return c;
}

char Lexer::next() {
    char c = stream.next();
    
    if (c) {
        offset += 1;
        text += c;
    }

    return c;
}

char Lexer::peek() {
    return stream.peek();
}

std::string Lexer::collect(char c, bool(*filter)(char)) {
    std::string out;
    
    if (c) {
        out += c;
    }

    while (filter(peek())) {
        out += next();
    }

    return out;
}

Range Lexer::here() {
    return { this, start, offset - start };
}
