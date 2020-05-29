#pragma once

#include <string>
#include <variant>

namespace ct
{
    enum class Keyword
    {

    };

    struct Token
    {
        enum {
            STRING,
            IDENT,
            KEYWORD,
            INTEGER,
            FLOAT,
            CHAR8,
            CHAR16,
            CHAR32,
        } type;

        std::variant<
            std::string,
            Keyword,
            uint64_t,
            double
        > data;
    };

    struct Lexer
    {
        Token next();
    };
}