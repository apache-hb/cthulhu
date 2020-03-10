#pragma once

#include <istream>
#include <string>
#include <memory>

#include "keywords.h"

namespace ctu
{
    struct Token
    {
        virtual ~Token() {}
        virtual std::string to_string() const = 0;
    };

    struct Key : Token
    {
        Key(Keyword k)
            : key(k)
        {}

        Keyword key;

        virtual std::string to_string() const override 
        {
#define KEYWORD(id, str) case Keyword::id: return "Key(" str ")";
#define OPERATOR(id, str) case Keyword::id: return "Key(" str ")";
#define RES_KEYWORD(id, str) case Keyword::id: return "Key(" str ")";
#define ASM_KEYWORD(id, str) case Keyword::id: return "Key(" str ")";

            switch(key)
            {
#include "keywords.inc"
            default: return "Key(kinvalid)";
            }
        }
    };

    struct Ident : Token
    {
        Ident(std::string i)
            : ident(i)
        {}

        std::string ident;

        virtual std::string to_string() const override 
        {
            return "Ident(" + ident + ")";
        }
    };

    struct Int : Token
    {
        Int(uint_fast64_t n)
            : num(n)
        {}

        uint_fast64_t num;

        virtual std::string to_string() const override 
        {
            return "Int(" + std::to_string(num) + ")";
        }
    };

    struct Float : Token
    {
        Float(double d)
            : num(d)
        {}

        double num;

        virtual std::string to_string() const override 
        {
            return "Float(" + std::to_string(num) + ")";
        }
    };

    struct String : Token
    {
        String(std::string buf)
            : str(buf)
        {}

        std::string str;

        virtual std::string to_string() const override 
        {
            return "String(" + str + ")";
        }
    };

    struct Char : Token
    {
        Char(char l)
            : c(l)
        {}

        char c;

        virtual std::string to_string() const override 
        {
            return std::string("Char(") + c + ")";
        }
    };

    struct Invalid : Token
    {
        Invalid(std::string r)
            : reason(r)
        {}

        std::string reason;

        virtual std::string to_string() const override
        {
            return "Invalid(" + reason + ")";
        }
    };

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
        Lexer(std::istream* input)
            : in(input)
        {
            tok = parse();
        }
        
        std::shared_ptr<Token> next();
        std::shared_ptr<Token> peek();
    private:

        int skip_whitespace(int i);

        std::shared_ptr<Token> parse();

        Keyword symbol(int c);

        std::shared_ptr<Token> alpha(int c);

        std::shared_ptr<Token> number(int c);

        String str();

        Char ch();

        char getchar(bool* b);

        std::shared_ptr<Token> hex();
        std::shared_ptr<Token> binary();

        std::shared_ptr<Token> tok;

        int nextc();
        int peekc();
        bool eatc(int c);

        std::istream* in;
    };

    struct Parser
    {

    };
}