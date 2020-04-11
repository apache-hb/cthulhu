#pragma once

#include <variant>

#include "utils/stream.h"

namespace ctu
{
#define KEY(id, _) id,
#define RES(id, _) id,
#define SOFT(id, _) id,
#define OP(id, _) id,

    enum class Keyword
    {
#include "keys.inc"
    };

    std::string to_string(Keyword key)
    {
#define KEY(id, str) if(Keyword::id == key) return str;
#define RES(id, str) if(Keyword::id == key) return str;
#define SOFT(id, str) if(Keyword::id == key) return str;
#define OP(id, str) if(Keyword::id == key) return str;
#include "keys.inc"
        return "invalid";
    }

    std::string to_string(std::string s)
    {
        return s;
    }

    std::string to_string(uint8_t s)
    {
        return std::to_string((int)s);
    }
};

#define TOKEN_T(name, type, wraps) struct name { name(type k_##wraps) : wraps(k_##wraps) {} \
        bool operator==(type k_##wraps) const { return wraps == k_##wraps; } \
        bool equals(type k_##wraps) const { return wraps == k_##wraps; } \
        std::string string() const { return to_string(wraps); } \
        type wraps; }

namespace ctu
{
    struct fpos
    {
        uint64_t dist = 0;
        uint64_t col = 0;
        uint64_t line = 0;
    };

    using namespace std;

    TOKEN_T(String, std::string, str);
    TOKEN_T(Char, uint8_t, sym);
    TOKEN_T(Ident, std::string, name);
    TOKEN_T(Int, uint64_t, num);
    TOKEN_T(Float, double, num);
    TOKEN_T(Key, Keyword, key);
    
    struct Eof
    {
        std::string string() const { return "EOF"; }
    };

    using TokenData = std::variant<
        String,
        Ident,
        Key,
        Char,
        Int,
        Float,
        Eof
    >;

    struct Token
    {
        template<typename T>
        Token(fpos p, T v)
            : pos(p)
            , data(v)
        {}

        fpos pos;

        TokenData data;
    };

    std::string to_string(Token tok)
    {
        return std::visit([](auto&& v) {
            return v.string();
        }, tok.data);
    }

    enum LexFlags
    {
        FNone = 0,
        FAttrib = 1 << 0
    };

    struct Lexer
    {
        Lexer(stream* i)
            : in(i)
        {}

        Token next(LexFlags flags = None)
        {
            return parse(flags);
        }

    private:

        uint8_t skip_whitespace()
        {
            auto c = get();
            while(isspace(c))
                c = get();

            return c;
        }

        uint8_t get()
        {
            auto c = in->next();

            if(c == '\n')
            {
                pos.col = 0;
                pos.line++;
            }
            else
            {
                pos.col++;
            }

            pos.dist++;

            return c;
        }

        bool consume(uint8_t c)
        {
            if(in->peek() == c)
            {
                get();
                return true;
            }

            return false;
        }

        Token parse(LexFlags flags)
        {
            auto c = skip_whitespace();

            while(c == '#')
                c = skip_whitespace();

            auto here = pos;

            switch(c)
            {
            case '~':
            case '!':
            case '%':
            case '^':
            case '&':
            case '*':
            case '(':
            case ')':
            case '-':
            case '+':
            case '=':
                if(consume('='))
                {
                    return Token(here, Keyword::_eq);
                }
                else if(consume('>'))
                {
                    return Token(here, Keyword::_arrow);
                }
                else
                {
                    printf("= is not a valid keyword\n");
                    std::exit(500);
                }
            case '{':
            case '}':
            case '[':
                if(flags & FAttrib)
                {
                    if(consume('['))
                        return Token(here, Keyword::_lsquare2);
                }
                return Token(here, Keyword::_lsquare);
            case ']':                
                if(flags & FAttrib)
                {
                    if(consume(']'))
                        return Token(here, Keyword::_rsquare2);
                }
                return Token(here, Keyword::_rsquare);
            case '|':
            case ':':
                return Token(here, consume(':') ? Keyword::_colon2 : Keyword::_colon);
            case '<':
            case '>':
            case ',':
            case '.':
            case '?':
            case '/':
            case '0':
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '8': case '9':


            case 'a': case 'b': case 'c':
            case 'd': case 'e': case 'f':
            case 'g': case 'h': case 'i':
            case 'j': case 'k': case 'l': 
            case 'm': case 'n': case 'o': 
            case 'p': case 'q': case 'r': 
            case 's': case 't': case 'u': 
            case 'v': case 'w': case 'x': 
            case 'y': case 'z': 
            case 'A': case 'B': case 'C':
            case 'D': case 'E': case 'F':
            case 'G': case 'H': case 'I':
            case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U':
            case 'V': case 'W': case 'X':
            case 'Y': case 'Z': 
            case '_':
                return Token(here, key_or_ident(c));

            case '\0':
                return Token(here, Eof());

            default:
                // error
                printf("oh no %c", c);
                std::exit(500);
            }
        }

        TokenData key_or_ident(uint8_t c)
        {
            std::string str;
            str += c;

            while(std::isalnum(in->peek()) || in->peek() == '_')
                str += get();

#define KEY(id, s) if(str == s) return Key(Keyword::id);
#include "keys.inc"

            return Ident(str);
        }

        stream* in;

        fpos pos;
    };
}