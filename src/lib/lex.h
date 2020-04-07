#pragma once

#include "utils/stream.h"

namespace ctu
{
    struct token
    {
        union
        {
            char* str;
        };
    };

    struct lexer
    {
        token next()
        {
            auto temp = tok;
            tok = parse();
            return temp;
        }

        token peek()
        {
            return tok;
        }

    private:
        uint8_t skip_whitespace()
        {
            auto c = in->next();
            while(isspace(c))
                c = in->next();

            return c;
        }

        token parse()
        {
            auto c = skip_whitespace();

            while(c == '#')
                c = skip_whitespace();

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
            case '{':
            case '}':
            case '[':
            case ']':
            case '|':
            case ':':
            case '<':
            case '>':
            case ',':
            case '.':
            case '?':
            case '/':
            case '0':
                if(in->peek() == 'x') 
                {
                    in->next();
                }
                else if(in->peek() == 'b') 
                {
                    in->peek();
                }
                else 
                {
                    // error
                }
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
            }
        }

        token tok;
        stream* in;
    };
}