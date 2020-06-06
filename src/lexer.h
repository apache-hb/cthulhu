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

        static Token invalid()
        {
            return Token{INVALID, UINT64_C(0)};
        }
    };

    const char* to_string(Keyword key)
    {
        switch (key)
        {
#define KEY(id, str) case Keyword::id: return str;
#define OP(id, str) case Keyword::id: return str;
#include "keywords.inc"
        default: return "uhhh";
        }
    }


    std::string to_string(Token tok)
    {
        switch (tok.type)
        {
        case Token::IDENT:
            return "Ident(" + std::get<std::string>(tok.data) + ")";
        case Token::KEYWORD:
            return std::string("Keyword(") + to_string(std::get<Keyword>(tok.data)) + ")";
        case Token::INVALID:
            return "Invalid()";
        case Token::END:
            return "End()";
        default:
            return "Other()";
        }
    }

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
                // single line comments
                if (consume('/'))
                {
                    // skip until the line end
                    skip([](char c) { return c != '\n'; });
                    // then get the next non-whitespace character
                    c = skip([](char c) { return isspace(c); });
                }
                else if (consume('*'))
                {
                    int depth = 1;
                    while (depth)
                    {
                        c = getc();
                        if (c == '/' && consume('*'))
                        {
                            depth++;
                        }
                        else if (c == '*' && consume('/'))
                        {
                            depth--;
                        }
                    }
                    c = skip([](char c) { return isspace(c); });
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
                    return Token::key(consume('=') ? Keyword::ASSIGN : (consume(':') ? Keyword::COLON2 : Keyword::COLON));
                case '.':
                    return Token::key(consume('.') ? (consume('.') ? Keyword::ELLIPSIS : Keyword::DOT2) : Keyword::DOT);
                case ';': return Token::key(Keyword::SEMICOLON);
                case ',': return Token::key(Keyword::COMMA);
                case '@': return Token::key(Keyword::AT);
#define KEY_EQ(C, E1, E2) case C: return Token::key(consume('=') ? Keyword::E1 : Keyword::E2)
                KEY_EQ('+', ADDEQ, ADD);
                KEY_EQ('-', SUBEQ, SUB);
                KEY_EQ('*', MULEQ, MUL);
                KEY_EQ('%', MODEQ, MOD);
                KEY_EQ('^', XOREQ, XOR);
                case '&':
                    return Token::key(consume('&') ? Keyword::AND : (consume('=') ? Keyword::BANDEQ : Keyword::BAND));
                case '|':
                    return Token::key(consume('|') ? Keyword::OR : (consume('=') ? Keyword::BOREQ : Keyword::BOR));
                case '!': return Token::key(consume('=') ? Keyword::NEQ : Keyword::NOT);
                case '=':
                    if (consume('='))
                        return Token::key(Keyword::EQ);
                    else { break; }
                        // error
                case '~': return Token::key(Keyword::BNOT);
                case '<':
                    if (consume('<'))
                        return Token::key(consume('=') ? Keyword::SHLEQ : Keyword::SHL);
                    else
                        return Token::key(consume('=') ? Keyword::LTE : Keyword::LT);
                case '>':
                    if (consume('>'))
                        return Token::key(consume('=') ? Keyword::SHREQ : Keyword::SHR);
                    else
                        return Token::key(consume('=') ? Keyword::GTE : Keyword::GT);

                case '?': return Token::key(Keyword::QUESTION);
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