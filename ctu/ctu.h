#pragma once

#include <istream>
#include <string>
#include <memory>
#include <map>
#include <variant>
#include <vector>

#include "keywords.h"

namespace ctu
{
    struct Key
    {
        Key(Keyword k)
            : key(k)
        {}

        Keyword key;
    };

    struct Ident
    {
        Ident(std::string i)
            : ident(i)
        {}

        std::string ident;
    };

    struct Int
    {
        Int(uint_fast64_t n)
            : num(n)
        {}

        uint_fast64_t num;
    };

    struct Float
    {
        Float(double d)
            : num(d)
        {}

        double num;
    };

    struct String
    {
        String(std::string buf)
            : str(buf)
        {}

        std::string str;
    };

    struct Char
    {
        Char(char l)
            : c(l)
        {}

        char c;
    };

    struct Invalid
    {
        Invalid(std::string r)
            : reason(r)
        {}

        std::string reason;
    };

    using Token = std::variant<Key, Ident, Int, Float, String, Char, Invalid>;

    struct FilePos
    {
        // line number
        uint_fast64_t line;

        // column number
        uint_fast64_t col;

        // distance into file
        uint_fast64_t dist;

        // the associated file
        std::istream* file;
    };

    struct Lexer
    {
        Lexer() = delete;
        Lexer(Lexer&) = delete;
        Lexer(Lexer&&) = delete;
        Lexer(std::istream* input)
            : in(input)
        {
            tok = parse();
        }
        
        Token next();
        Token peek();
    private:

        int skip_whitespace(int i);

        Token parse();

        Keyword symbol(int c);

        Token alpha(int c);

        Token number(int c);

        String str();

        Char ch();

        char getchar(bool* b);

        Token hex();
        Token binary();

        Token tok = Invalid("nop");

        int nextc();
        int peekc();
        bool eatc(int c);

        std::istream* in;
    };

    struct Expr
    {

    };

    struct UnaryExpr : Expr
    {
        Keyword op;
        Expr expr;
    };

    struct BinaryExpr : Expr
    {
        Keyword op;
        Expr lhs;
        Expr rhs;
    };

    struct Parser
    {
        Parser(Lexer* l)
            : lex(l)
        {}

    private:

        Lexer* lex;
    };
}