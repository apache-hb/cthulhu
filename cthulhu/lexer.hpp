#pragma once

#include <stddef.h>
#include <stdint.h>

#include "nodes.hpp"

//
// main lexer and stream interface
//

namespace cthulhu {
    static constexpr char32_t END = char32_t(-1);

    struct StreamHandle {
        virtual ~StreamHandle() {}
        virtual char32_t next() = 0;
    };

    struct Stream {
        Stream(StreamHandle* handle);

        char32_t next();
        char32_t peek();
    private:
        StreamHandle* handle;
        char32_t lookahead;
    };

    struct Location {
        Lexer* lexer;
        size_t offset;
        size_t line;
        size_t column;

        Location(Lexer* lexer, size_t offset, size_t line, size_t column);

        Range* to(Location other);
    };

    struct Range : Location {
        size_t length;

        Range(Lexer* lexer, size_t offset, size_t line, size_t column, size_t length);
    };

    struct Lexer {
        Lexer(Stream stream, utf8::string name = u8"input")
            : name(name)
            , stream(stream)
            , here(Location(this, 0, 0, 0))
        { }

        Token* read();

        utf8::string slice(Range* range);
        utf8::string line(size_t it);

        const utf8::string name;

    private:
        utf8::string collect(char32_t start, bool(*filter)(char32_t));

        char32_t skip();
        char32_t next();
        char32_t peek();
        char32_t escape();
        bool eat(char32_t c);

        Stream stream;
        Location here;
        utf8::string text = "";
        int depth = 0;
    };
}
