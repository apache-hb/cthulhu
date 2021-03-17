#include "lexer.h"

#include <iostream>

namespace {
    bool ident1(char c) {
        return isalpha(c) || c == '_';
    }

    bool ident2(char c) {
        return isalnum(c) || c == '_';
    }

    KeyMap CORE = {
#define KEY(id, str) { str, Key::id },
#include "keys.inc"
    };

    KeyMap ASM = {
#define ASM(id, str) { str, Key::id },
#include "keys.inc"
    };
}

Lexer::Lexer(StreamHandle* handle, std::string name)
    : stream(handle)
    , keys(&CORE)
    , name(name)
{ }

Token Lexer::read() {
    char c = skip();
    if (c == 0) {
        return Token(here());
    } else if (c == 'R') {
        return rstring();
    } else if (c == '"') {
        return string();
    } else if (ident1(c)) {
        return ident(c);
    } else {
        return symbol(c);
    }
}

Token Lexer::ident(char c) {
    auto id = collect(c, ident2);
    auto iter = keys->find(id);

    if (iter != keys->end()) {
        return Token(here(), Token::KEY, { .key = iter->second });
    } else {
        return Token(here(), Token::IDENT, { .ident = pool.intern(id) });
    }
}

Token Lexer::symbol(char c) {
    switch (c) {
    case '+': return Token(here(), Token::KEY, { .key = eat('=') ? Key::ADDEQ : Key::ADD });
    case '-': return Token(here(), Token::KEY, { .key = eat('=') ? Key::SUBEQ : Key::SUB });
    case '!': {
        if (eat('<')) {
            depth++;
            return Token(here(), Token::KEY, { .key = Key::BEGIN });
        } else {
            return Token(here(), Token::KEY, { .key = eat('=') ? Key::NEQ : Key::NOT });
        }
    }
    case '>': {
        if (depth > 0) {
            depth--;
            return Token(here(), Token::KEY, { .key = Key::END });
        } else if (eat('>')) {
            return Token(here(), Token::KEY, { .key = eat('=') ? Key::SHREQ : Key::SHR });
        } else {
            return Token(here(), Token::KEY, { .key = eat('=') ? Key::GTE : Key::GT });
        }
    }
    case '<': {
        if (eat('<')) {
            return Token(here(), Token::KEY, { .key = eat('=') ? Key::SHLEQ : Key::SHL });
        } else {
            return Token(here(), Token::KEY, { .key = eat('=') ? Key::LTE : Key::LT });
        }
    }
    default: return Token(here());
    }
}

Token Lexer::rstring() {
    if (!eat('"')) {
        return ident('R');
    }

    // collect the string delimiter
    auto delim = ")" + collect(0, [](char c) { return c != '('; }) + '"';

    std::cout << delim << std::endl;

    if (!eat('(')) {
        return Token(here(), Token::ERROR_STRING_EOF);
    }

    std::string str;

    while (!str.ends_with(delim)) {
        if (char c = next(); c != 0) {
            str += c;
        } else {
            return Token(here(), Token::ERROR_STRING_EOF);
        }
    }

    str = str.substr(0, str.size() - delim.length());

    return Token(here(), Token::STRING, { .string = pool.intern(str) });
}

Token Lexer::string() {
    std::string str;

    while (true) {
        char c = next();
        
        if (c == '"') {
            break;
        } else if (c == '\n') {
            return Token(here(), Token::ERROR_STRING_LINE);
        } else if (c == 0) {
            return Token(here(), Token::ERROR_STRING_EOF);
        }

        str += c;
    }

    return Token(here(), Token::STRING, { .string = pool.intern(str) });
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

    // figure out where we are on this line
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

bool Lexer::eat(char c) {
    if (peek() == c) {
        next();
        return true;
    }
    return false;
}

std::string Lexer::collect(char c, bool(*filter)(char)) {
    std::string out;
    
    if (c) {
        out += c;
    }

    while (filter(peek()) && peek() != 0) {
        out += next();
    }

    return out;
}

Range Lexer::here() {
    return { this, start, offset - start };
}
