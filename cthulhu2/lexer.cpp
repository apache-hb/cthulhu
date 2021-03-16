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

std::string Lexer::lines(size_t first, size_t length) {
    size_t len = length;
    size_t front = first;

    // backtrack to the start of this line
    while (front > 0 && text[front] != '\n') {
        front--;
        len++;
    }

    // find the end of this current line
    while (front + len < text.length() && text[front + len] != '\n') {
        len++;
    }

    return text.substr(front, len);
}

Location Lexer::location(size_t first) {
    size_t line = 1;
    size_t column = 0;
    size_t front = 0;

    // now figure out where we are on this line
    while (front < first && front < text.length()) {
        char c = text[front++];
        if (c == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
    }

    return { line, column };
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
