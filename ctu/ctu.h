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

    struct Eof { };

    using Token = std::variant<Key, Ident, Int, Float, String, Char, Invalid, Eof>;

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

    // type = struct | tuple | union | variant | enum | ptr | array | typename | builtin
    struct Type { };

    // struct = `{` [struct-body] `}`
    struct Struct : Type
    {
        // struct-body = ident `:` type [`,` struct-body]
        std::map<std::string, Type> fields;
    };

    // tuple = `(` [tuple-body] `)`
    struct Tuple : Type
    {
        // tuple-body = type [`,` tuple-body]
        std::vector<Type> fields;
    };

    enum class BuiltinType
    {
        u8,
        u16,
        u32,
        u64,
        u128,
        i8,
        i16,
        i32,
        i64,
        i128,
        b,
        udefault,
        idefault,
        f32,
        f64,
    };

    struct Builtin : Type
    {
        BuiltinType type;
    };

    struct Func
    {
        std::string name;

        std::map<std::string, Type> args;
        Type ret;

        // TODO: body
    };

    // typedef = `type` ident `=` type
    struct TypeDef
    {
        std::string name;
        Type type;
    };

    struct Global
    {
        std::string name;
        // TODO: value
    };

    using Body = std::variant<Func, TypeDef, Global>;

    using AST = std::vector<Body>;

    struct Parser
    {
        Parser(Lexer* l)
            : lex(l)
        {}

        AST parse();

    private:
        Type type();
        std::map<std::string, Type> func_args();
        Func func();
        TypeDef typedecl();
        Global global();

        template<typename T>
        T next()
        {
            auto tok = lex->next();
            if(auto t = std::get_if<T>(&tok))
            {
                return *t;
            }

            // error
        }

        void expect(Keyword k)
        {
            if(next<Key>().key != k)
            {
                // error
            }
        }

        Lexer* lex;
    };
}