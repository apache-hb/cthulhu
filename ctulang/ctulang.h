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

        // the length of the token
        int length() const;

        keyword key() const { return type == token_type::keyword ? std::get<keyword>(data) : keyword::kinvalid; }
        std::string ident() const { return type == token_type::ident ? std::get<std::string>(data) : ""; }
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
        char lex_char(bool* end);

        int getc();
        int peekc();

        std::istream* stream;
        std::string filename;
        token tok;

        uint_fast64_t line = 0;
        uint_fast64_t col = 0;
        uint_fast64_t pos = 0;
    };

    namespace ast
    {
        using path = std::vector<std::string>;

        struct expr {};

        struct attribute
        {
            path name;

            std::vector<expr> positional;
            std::map<std::string, expr> args;
        };

        struct type 
        {
            std::vector<attribute> attribs;
        };

        struct func 
        {
            std::vector<attribute> attribs;
        };

        struct typetable
        {
            type* get(std::size_t id)
            {
                return types[id];
            }

            std::vector<type*> types;

            ~typetable()
            {
                for(auto* type : types)
                    delete type;
            }
        };

        struct functable
        {
            func* get(const std::string& name)
            {
                return funcs[name];
            }

            std::map<std::string, func*> funcs;

            ~functable()
            {
                for(auto [key, val] : funcs)
                    delete val;
            }
        };

        struct ast
        {
            path mod;
            std::vector<path> imports;
        };
    };

    ast::ast parse(lexer lex, ast::typetable* types, ast::functable* funcs);
}