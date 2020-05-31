#pragma once

#include <string>
#include <variant>
#include <cctype>

#include "crc32.h"

namespace ct
{
    enum class Keyword
    {
#define KEY(id, str) id,
#define OP(id, str) id,
#define ASM(id, str) id,
#include "keywords.inc"
    };

    struct Token
    {
        enum {
            STRING,
            IDENT,
            KEYWORD,
            INT,
            FLOAT,
            CHAR8,
            END,
            INVALID
        } type;

        std::variant<
            std::string,
            Keyword,
            uint64_t,
            double
        > data;

        static Token ident(std::string&& str)
        {
            return Token{IDENT, str};
        }

        static Token key(Keyword k)
        {
            return Token{KEYWORD, k};
        }

        static Token integer(uint64_t n)
        {
            return Token{INT, n};
        }

        static Token decimal(double d)
        {
            return Token{FLOAT, d};
        }
    };

    Token keyword(std::string&& str)
    {
        switch (crc32(str))
        {
#define KEY(id, str) case crc32(str): return Token::key(Keyword::id);
#include "keywords.inc"
        default: return Token::ident(std::move(str));
        }
    }

    struct Lexer
    {
        Lexer(FILE *in)
            : stream(in)
            , ahead(fgetc(in))
        { }

        Token next()
        {
            char c = skip([](char c) { return isspace(c); });

            // skip comments
            while (c == '/')
            {
                if (consume('/'))
                {
                    c = skip([](char c) { return c != '\n'; });
                }
                else if (consume('*'))
                {
                    int depth = 1;
                    while ((c = getc()))
                    {
                        if (c == '/' && consume('*'))
                        {
                            depth++;
                        }
                        else if (c == '*' && consume('/'))
                        {
                            depth--;
                        }
                    }
                }
                else
                {
                    return Token::key(consume('=') ? Keyword::DIVEQ : Keyword::DIV);
                }
            }

            if (c == EOF)
            {
                return Token{Token::END, UINT64_C(0)};
            }
            else if (isalpha(c))
            {
                std::string buffer = {c};
                while (true)
                {
                    c = peekc();
                    if (isalnum(c) || c == '_')
                    {
                        buffer += getc();
                    }
                    else
                    {
                        break;
                    }
                }

                return keyword(std::move(buffer));
            }
            else if (isdigit(c))
            {

            }
            else if (c == '\'')
            {

            }
            else if (c == '"')
            {

            }
            else
            {
                switch (c)
                {
                case '(': return Token::key(Keyword::LPAREN);
                case ')': return Token::key(Keyword::RPAREN);
                case '[': return Token::key(Keyword::LSQUARE);
                case ']': return Token::key(Keyword::RSQUARE);
                case '{': return Token::key(Keyword::LBRACE);
                case '}': return Token::key(Keyword::RBRACE);
                case ':':
                    if (consume('='))
                        return Token::key(Keyword::ASSIGN);
                    else if (consume(':'))
                        return Token::key(Keyword::COLON2);
                    else 
                        return Token::key(Keyword::COLON);
                case '.':
                    if (consume('.'))
                        if (consume('.'))
                            return Token::key(Keyword::ELLIPSIS);
                        else 
                            return Token::key(Keyword::DOT2);
                    else
                        return Token::key(Keyword::DOT);
                case ';':
                    return Token::key(Keyword::SEMICOLON);
                case ',':
                    return Token::key(Keyword::SEMICOLON);
                case '@':
                    return Token::key(Keyword::AT);
                default:
                    break;
                }
            }

            printf("eek\n");
            exit(-1);
        }

        template<typename F>
        char skip(F&& func)
        {
            char c = getc();
            while (func(c))
                c = getc();

            return c;
        }

        char getc()
        {
            char c = ahead;
            ahead = fgetc(stream);
            return c;
        }

        char peekc()
        {
            return ahead;
        }

        bool consume(char c)
        {
            if (peekc() == c)
            {
                getc();
                return true;
            }

            return false;
        }

        FILE *stream;
        char ahead;
    };
}