#pragma once

#include "token.h"
#include "stream.h"

struct Lexer {
    Lexer(StreamHandle* handle, std::string name = "unnamed")
        : stream(handle)
        , name(name)
    { }

    Token read();

    // collect the full line text for whatever line is at offset to whatever line
    // offset + length ends on
    std::string lines(size_t first, size_t length);
    Location location(size_t first);

    char skip();
    char next();
    char peek();
    std::string collect(char c, bool(*filter)(char));

    Range here();

    size_t start = 0;
    size_t offset = 0;
    Stream stream;
    std::string name;
    std::string text;
};
