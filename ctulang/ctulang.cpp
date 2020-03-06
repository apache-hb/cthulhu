#include "ctulang.h"

#include <cctype>

#include <iostream>

#include <thirdparty/termcolor.hpp>

namespace ctu
{
    int lexer::getc()
    {
        int c = stream->get();

        if(c == '\n')
        {
            line++;
            col = 0;
        }
        else
        {
            col++;
        }

        pos++;

        return c;
    }

    int lexer::peekc()
    {
        return stream->peek();
    }

    token lexer::peek()
    {
        return tok;
    }

    token lexer::next()
    {
        auto temp = tok;
        tok = parse();
        return temp;
    }

    token lexer::parse()
    {
        int c = getc();

        // remove whitespace
        while(std::isspace(c))
            c = getc();

        // remove comments
        if(c == '#')
        {
            // all comments are single line so we skip until the line ends
            while(c != '\n')
            {
                c = getc();
            }

            // handle multiple comments
            return parse();
        }
        
        if(std::isdigit(c) || c == '.')
        {
            // must be a number
            return lex_number(c);
        }
        else if(std::isalpha(c) || c == '_')
        {
            // can be either a keyword or an identifier
            return lex_alpha(c);
        }
        else if(c == '"')
        {
            // is either a string
            return lex_string();
        }
        else if(c == '\'')
        {
            return { token_type::character, lex_char(), line, col };
        }
        else
        {
            // must be symbolic, i.e an operator like + or -
            return lex_symbol(c);
        }
    }

    token lexer::lex_string()
    {
        std::string buffer = "";

        for(;;)
        {
            char n = lex_char();
            buffer += n;
        }

        return { token_type::string, buffer, line, col };
    }

    char lexer::lex_char()
    {
        int c = getc();

        if(c == '\\')
        {
            // is an escaped charater

            switch(getc())
            {
            case '\\': return '\\';
            case '\n': return '\n';
            case '\r': return '\r';
            case '0': return '\0';
            case '\t': return '\t';
            case '"': return '"';
            case '\'': return '\'';
            default:
                error({
                    stream,
                    filename,

                    pos,
                    line,
                    col,
                    1,

                    "invalid string escape",
                    'E',
                    0
                });
                break;
            }
        }

        std::abort();
    }

    token lexer::lex_number(int c)
    {
        std::abort();
    }

    token lexer::lex_alpha(int c)
    {
        std::abort();
    }

    token lexer::lex_symbol(int c)
    {
        switch(c)
        {
        case '[':
        case ']':
        
        case '(':
        case ')':

        case '{':
        case '}':


        case ':':

        case ',':
        case '.':
        case '?':
        case '@':
        case '!':
        case '~':
        case '^':
        case '&':
        case '*':
        case '-':
        case '=':
        case '+':
        case '|':
        case '<':
        case '>':
        case '/':
        case '%':
        default:
            error({
                stream,
                filename,

                pos,
                line,
                col,
                1,

                "invalid operator",
                'E',
                0
            });
            break;
        }
    }

    [[noreturn]]
    void error(const errordata& data)
    {
        std::cout << termcolor::red << "error" << termcolor::reset << "[" << data.prefix << data.id << "]: " << data.message << std::endl
                  << "--> " << data.filename << ":" << data.line << ":" << data.col << std::endl;

        std::string l = std::to_string(data.line);

        data.source->seekg(data.pos);

        std::string line = "";

        int c = data.source->get();
        while(c != '\n')
        {
            line += c;
            c = data.source->get();
        }

        std::cout << std::string(l.size() + 2, ' ') + "|" << std::endl
                  << " " << l << " |" << line << std::endl
                  << std::string(l.size() + 2, ' ') + "|" << std::string(data.col, ' ') << std::string(data.len, '^') << std::endl;

        std::exit(1);
    }
}