#pragma once

#include "fwd.hpp"

#include <stddef.h>

namespace cthulhu {
    struct Range {
        Range(Lexer* lexer);

    private:
        Lexer* lexer;
        size_t offset;
        size_t line;
        size_t column;
        size_t length;
    };

    enum struct Key {

    };

    struct Token {
        enum Type {
            IDENT,
            KEY,
            STRING,
            INT,
            CHAR,
            END
        };

        bool is(Type other) const;

        Token();

    private:
        struct Ident {
            utf8::string ident;
        };

        struct String {
            utf8::string prefix;
            utf8::string str;
        };

        struct Int {
            utf8::string suffix;
        };

        struct Char {
            c32 c;
        };

        struct Keyword {
            Key key;
        };

        union Data {
            Data();
            ~Data();

            Ident id;
            String str;
            Int num;
            Char c;
            Key key;
        };

        Range where;
        Type type;
        Data data;
    };
}
