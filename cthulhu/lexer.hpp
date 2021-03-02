#pragma once

#include "stream.hpp"
#include "token.hpp"
#include "pool.hpp"
#include "fwd.hpp"

namespace cthulhu {
    struct Lexer {
        Lexer(Stream stream, utf8::string name);

        Token read();

    private:
        c32 next();
        c32 peek();
        c32 skip();
        bool eat(c32 c);

        // out source stream
        Stream stream;

        // our current position
        Range here;

        // name of the lexer
        utf8::string name;

        // all currently read text
        utf8::string text;

        // all strings that have been lexed
        StringPool strings;

        // all idents that have been lexed
        StringPool idents;
    };
}