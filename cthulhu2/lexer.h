#pragma once

#include "token.h"
#include "stream.h"
#include "util.h"

using KeyMap = std::unordered_map<std::string, Key>;

struct Lexer {
    Lexer(StreamHandle* handle, std::string name = "unnamed", Pool* pool = nullptr);

    Token read();
    Token grab();

    // collect the full line text for whatever line is at offset to whatever line
    // offset + length ends on
    std::string lines(size_t first, size_t length);
    Location location(size_t first);

    Token ident(char c);
    Token string();
    Token rstring();
    Token letter();
    Token number(char c);
    Token symbol(char c);

    enum Base {
        BASE10,
        BASE16
    };

    void encode(std::string* out, Base base);
    
    struct StringResult {
        Token::Type error;
        Range range;
        const std::string* string;
    };

    StringResult letters(char end);

    char skip();
    char next();
    char peek();
    bool eat(char c);
    std::string collect(char c, bool(*filter)(char));

    Range here();

    size_t depth = 0;
    size_t start = 0;
    size_t offset = 0;
    Stream stream;
    Pool* pool;
    KeyMap* keys;
    std::string name;
    std::string text;

    // we save this token for error handling
    Token saved;
};
