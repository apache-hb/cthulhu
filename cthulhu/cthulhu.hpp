#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <tinyutf8/tinyutf8.h>

namespace cthulhu {
    using namespace std;

    namespace utf8 = tiny_utf8;

    struct Stream;
    struct Lexer;
    struct Location;
    struct Range;
    struct Token;
    struct Ident;
    struct Key;
    struct Int;
    struct String;
    struct End;

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
        Lexer(Stream stream);

        Token* read();

        utf8::string slice(Range* range);

    private:
        utf8::string collect(char32_t start, bool(*filter)(char32_t));

        char32_t skip();
        char32_t next();
        char32_t peek();
        bool eat(char32_t c);

        Stream stream;
        Location here;
        utf8::string text;
        int depth;
    };

    struct Token {
        Range* range;

        virtual ~Token() {}
    };

    struct Ident : Token {
        Ident(utf8::string id);

        bool operator==(const Ident& other) const;

    private:
        utf8::string ident;
    };

    struct Key : Token {
#define KEY(id, _) id,
#define OP(id, _) id,
        enum Word { 
#include "keys.inc"
            INVALID
        } key;

        Key(Word word);

        bool operator==(const Key& other) const;
    };

    struct Int : Token {
        Int(uint64_t num, utf8::string str = u8"");

        bool operator==(const Int& other) const;

    private:
        uint64_t num;
        utf8::string suffix;
    };

    struct String : Token {
        String(utf8::string str);

        bool operator==(const String& other) const;

    private:
        utf8::string str;
    };

    struct End : Token {
        End() { }

        bool operator==(const End&) const { return true; }
    };

    struct Error : Token {

    };
}
