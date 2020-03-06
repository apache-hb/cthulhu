#pragma once

#include <string>
#include <variant>
#include <memory>
#include <map>
#include <vector>
#include <istream>

namespace ctu
{
    enum class token_type
    {
        string,
        character,
        ident,
        integer,
        number,
        keyword,
        eof,
    };

#define KEYWORD(id, str) id,
#define OPERATOR(id, str) id,

    enum class keyword
    {
#include "keywords.inc"
    };
    
    struct token
    {
        using data_type = std::variant<
            std::string,
            int_fast64_t,
            char,
            double,
            keyword
        >;

        token_type type;
        data_type data;

        // the line this token appears on
        uint_fast64_t line;

        // the column this token starts on
        uint_fast64_t col;

        // the length of the token
        int length() const;
    };

    struct lexer
    {
        lexer(std::istream* in, std::string name = "<undefined>")
            : stream(in)
            , filename(name)
        {
            tok = parse();
        }

        token peek();
        token next();

    private:
        token parse();

        token lex_number(int c);
        token lex_alpha(int c);
        token lex_symbol(int c);
        token lex_string();
        char lex_char();

        int getc();
        int peekc();

        std::istream* stream;
        std::string filename;
        token tok;

        uint_fast64_t line = 0;
        uint_fast64_t col = 0;
        uint_fast64_t pos = 0;
    };

    struct errordata
    {
        std::istream* source;
        std::string filename;

        uint_fast64_t pos;
        uint_fast64_t line;
        uint_fast64_t col;
        uint_fast64_t len;

        std::string message;
        char prefix;
        int id;
    };

    [[noreturn]]
    void error(const errordata& data);
}