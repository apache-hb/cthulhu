#include "ctulang.h"

#include <cctype>

#include <iostream>

#include <thirdparty/termcolor.hpp>

namespace ctu
{
    int token::length() const
    {
        return std::visit([](auto&& arg) -> size_t {
            using type = std::decay_t<decltype(arg)>;

            if constexpr(std::is_same_v<type, std::string>)
            {
                return arg.size();
            }
            else if constexpr(std::is_same_v<type, int_fast64_t>)
            {
                return std::to_string(arg).size();
            }
            else if constexpr(std::is_same_v<type, double>)
            {
                return std::to_string(arg).size();
            }
            else if constexpr(std::is_same_v<type, keyword>)
            {
#define KEYWORD(id, str) case keyword::id: return strlen(str);
#define OPERATOR(id, str) case keyword::id: return strlen(str);
                switch(arg)
                {
#include "keywords.inc"
                default: return 1;
                }
            }
            else
            {
                return 1;
            }
        }, data);
    }

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
            bool stub;
            return { token_type::character, lex_char(&stub) };
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

        bool complete = false;

        while(!complete)
        {
            char n = lex_char(&complete);
            buffer += n;
        }

        return { token_type::string, buffer };
    }

    char lexer::lex_char(bool* end)
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
                // error
                std::abort();
                break;
            }
        }

        if(c == '"')
            *end = true;

        return c;
    }

    token lexer::lex_number(int c)
    {
        if(c == '0' && peekc() == 'x')
        {
            getc();
            std::string buffer;

            while(std::isxdigit(peekc()))
                buffer += getc();

            return { token_type::integer, std::stoll(buffer, 0, 16) };
        }
        else if(c == '0' && peekc() == 'b')
        {
            getc();
            std::string buffer;

            while(peekc() == '0' || peekc() == '1')
                buffer += getc();

            return { token_type::integer, std::stoll(buffer, 0, 2) };
        }
        else
        {
            std::string buffer;
            buffer += c;

            bool decimal = false;

            while(std::isdigit(peekc()) || peekc() == '.')
            {
                if(peekc() == '.')
                    decimal = true;

                buffer += getc();
            }

            if(decimal)
                return { token_type::number, std::stod(buffer) };
            else
                return { token_type::integer, std::stoll(buffer) };
            
        }
    }

    token lexer::lex_alpha(int c)
    {
        std::string buffer;
        buffer += c;

        while(std::isalnum(peekc()) || c == '_')
            buffer += getc();

#define KEYWORD(id, str) if(buffer == str) return { token_type::keyword, keyword::id };

#include "keywords.inc"

        return { token_type::ident, buffer };
    }

    token lexer::lex_symbol(int c)
    {
        switch(c)
        {
        case '[':
            if(peekc() == '[')
            {
                getc();
                return { token_type::keyword, keyword::klsquare2 };
            }
            return { token_type::keyword, keyword::klsquare };

        case ']':
            if(peekc() == ']')
            {
                getc();
                return { token_type::keyword, keyword::krsquare2 };
            }
            return { token_type::keyword, keyword::krsquare };
        
        case '(':
            return { token_type::keyword, keyword::klparen };
        case ')':
            return { token_type::keyword, keyword::krparen };

        case '{':
            return { token_type::keyword, keyword::klbrace };
        case '}':
            return { token_type::keyword, keyword::krbrace };

        case ':':
            if(peekc() == ':')
            {
                getc();
                return { token_type::keyword, keyword::kcolon2 };
            }
            else if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kassign };
            }

            return { token_type::keyword, keyword::kcolon };
        
        case ',':
            return { token_type::keyword, keyword::kcomma };

        case '.':
            return { token_type::keyword, keyword::kdot };

        case '?':
            return { token_type::keyword, keyword::kquestion };

        case '@':
            return { token_type::keyword, keyword::kat };

        case '!':
            return { token_type::keyword, keyword::knot };

        case '~':
            return { token_type::keyword, keyword::kbitnot };

        case '^':
            if(peekc() == '=')
            {
                return { token_type::keyword, keyword::kbitxoreq };
            }
            return { token_type::keyword, keyword::kbitxor };

        case '&':
            if(peekc() == '&')
            {
                getc();
                return { token_type::keyword, keyword::kand };
            }
            else if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kbitandeq };
            }
            return { token_type::keyword, keyword::kbitand };

        case '*':
            if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kmuleq };
            }
            return { token_type::keyword, keyword::kmul };

        case '-':
            if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::ksubeq };
            }
            return { token_type::keyword, keyword::ksub };

        case '=':
            return { token_type::keyword, keyword::keq };

        case '+':
            if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kaddeq };
            }
            return { token_type::keyword, keyword::kadd };

        case '|':
            if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kbitoreq };
            }
            else if(peekc() == '|')
            {
                getc();
                return { token_type::keyword, keyword::kor };
            }
            return { token_type::keyword, keyword::kbitor };

        case '<':
            if(peekc() == '<')
            {
                getc();
                if(peekc() == '=')
                {
                    getc();
                    return { token_type::keyword, keyword::kshleq };
                }
                return { token_type::keyword, keyword::kshl };
            }
            else if(peekc() == '=')
            {
                return { token_type::keyword, keyword::klte };
            }
            return { token_type::keyword, keyword::klt };

        case '>':
            if(peekc() == '>')
            {
                getc();
                if(peekc() == '=')
                {
                    getc();
                    return { token_type::keyword, keyword::kshreq };
                }
                return { token_type::keyword, keyword::kshr };
            }
            else if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kgte };
            }
            return { token_type::keyword, keyword::kgt };

        case '/':
            if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kdiveq };
            }
            return { token_type::keyword, keyword::kdiv };

        case '%':
            if(peekc() == '=')
            {
                getc();
                return { token_type::keyword, keyword::kmodeq };
            }
            return { token_type::keyword, keyword::kmod };

        default:
            // error
            std::abort();
            break;
        }
    }

    struct parser
    {
        lexer lex;

        bool consume(keyword k)
        {
            if(lex.peek().key() == k)
            {
                lex.next();
                return true;
            }

            return false;
        }

        void expect(keyword k)
        {
            if(lex.next().key() != k)
            {
                // error
            }
        }

        std::string ident()
        {
            auto str = lex.next().ident();

            if(str.empty())
            {
                // todo: error
            }

            return str;
        }

        ast::path dotted_name()
        {
            ast::path ret;

            // dotted-name-decl: ident [`::` dotted-name-decl]
            do { ret.push_back(ident()); } while(consume(keyword::kcolon2));

            return ret;
        }

        std::vector<ast::attribute> attributes;

        void parse_attrib_args(ast::attribute* attrib)
        {

        }

        void parse_body()
        {
            for(;;)
            {
                attributes.clear();

                // is an attribute
                while(consume(keyword::klsquare2))
                {
                    ast::attribute attrib;
                    attrib.name = dotted_name();

                    // has args
                    if(consume(keyword::klparen))
                    {
                        parse_attrib_args(&attrib);
                        expect(keyword::krparen);
                    }

                    expect(keyword::krsquare2);
                    attributes.push_back(attrib);
                }

                // function
                if(consume(keyword::kdef))
                {

                }
            }
        }
    };

    ast::ast parse(
        lexer lex,
        ast::typetable* types,
        ast::functable* funcs
    )
    {
        ast::ast ret;
        parser parse = { lex };

        if(parse.consume(keyword::kmodule))
            ret.mod = parse.dotted_name();

        while(parse.consume(keyword::kimport))
            ret.imports.push_back(parse.dotted_name());

        parse.parse_body();

        return ret;
    }
}