#pragma once

#include <istream>
#include <string>
#include <memory>
#include <map>
#include <vector>

#include "keywords.h"

namespace ctu
{
    enum class TokenType
    {
        key,
        ident,
        string,
        integer,
        number,
        character,
        invalid
    };

    struct Token
    {
        virtual ~Token() {}
        virtual std::string to_string() const = 0;
        virtual TokenType type() const = 0;
    };

    struct Key : Token
    {
        Key(Keyword k)
            : key(k)
        {}

        Keyword key;

        virtual TokenType type() const override { return TokenType::key; }

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

        virtual TokenType type() const override { return TokenType::ident; }

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

        virtual TokenType type() const override { return TokenType::integer; }

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

        virtual TokenType type() const override { return TokenType::number; }

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

        virtual TokenType type() const override { return TokenType::string; }

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

        virtual TokenType type() const override { return TokenType::character; }

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

        virtual TokenType type() const override { return TokenType::invalid; }

        virtual std::string to_string() const override
        {
            return "Invalid(" + reason + ")";
        }
    };

    template<typename T>
    TokenType tok_type();

    template<> inline TokenType tok_type<Key>() { return TokenType::key; }
    template<> inline TokenType tok_type<Ident>() { return TokenType::ident; }
    template<> inline TokenType tok_type<String>() { return TokenType::string; }
    template<> inline TokenType tok_type<Char>() { return TokenType::character; }
    template<> inline TokenType tok_type<Float>() { return TokenType::number; }
    template<> inline TokenType tok_type<Int>() { return TokenType::integer; }
    template<> inline TokenType tok_type<Invalid>() { return TokenType::invalid; }

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

    struct Type
    {
        virtual ~Type() {}

        virtual std::string to_string() const = 0;
    };

    enum class BuiltinType
    {
        u8,
        u16,
        u32,
        u64,
        u128,
        ufast,

        i8,
        i16,
        i32,
        i64,
        i128,
        ifast,

        b8,
        b16,
        b32,
        b64,
        bfast,

        f32,
        f64,

        char8,
        char16,
        char32,

        str8,
        str16,
        str32
    };

    struct Builtin : Type
    {
        Builtin(BuiltinType t)
            : type(t)
        {}

        BuiltinType type;

        virtual std::string to_string() const override { return "Builtin()"; }
    };

    struct Struct : Type
    {
        std::map<std::string, Type> fields;

        virtual std::string to_string() const override { return "Struct()"; }
    };

    struct Tuple : Type
    {
        std::vector<Type> fields;

        virtual std::string to_string() const override { return "Tuple()"; }
    };

    struct Union : Type
    {
        std::map<std::string, Type> fields;

        virtual std::string to_string() const override { return "Union()"; }
    };

    struct Variant : Type
    {
        std::shared_ptr<Type> backing;
        std::map<std::string, Type> fields;

        virtual std::string to_string() const override { return "Variant()"; }
    };

    struct Ptr : Type
    {
        std::shared_ptr<Type> to;

        virtual std::string to_string() const override { return "Ptr()"; }
    };

    struct Name : Type
    {
        Name(std::string n)
            : name(n)
        {}

        std::string name;

        virtual std::string to_string() const override { return "Name()"; }
    };

    struct Func
    {
        std::string name;
    };

    struct Parser
    {
        Parser(Lexer* l)
            : lex(l)
        {}

        

    private:

        std::shared_ptr<Type> type();

        template<typename T>
        std::shared_ptr<T> expect()
        {
            auto tok = lex->next();

            if(tok_type<T>() != tok->type())
            {
                // error
            }

            return tok;
        }

        Lexer* lex;
    };
}