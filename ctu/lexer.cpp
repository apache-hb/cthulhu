#include "ctu.h"

namespace ctu
{
    Token Lexer::next()
    {
        auto temp = tok;
        tok = parse();
        return temp;
    }

    Token Lexer::peek()
    {
        return tok;
    }

    Keyword Lexer::symbol(int c)
    {
        switch(c)
        {
        case '~':
            return Keyword::kbitnot;
        case '!':
            return eatc('=') ? Keyword::kneq : Keyword::knot;
        case '%':
            return eatc('=') ? Keyword::kmodeq : Keyword::kmod;
        case '^':
            return eatc('=') ? Keyword::kbitxoreq : Keyword::kbitxor;
        case '&':
            return eatc('&') ? Keyword::kand : eatc('=') ? Keyword::kbitandeq : Keyword::kbitand;
        case '*':
            return eatc('=') ? Keyword::kmuleq : Keyword::kmul;
        case '(':
            return Keyword::klparen;
        case ')':
            return Keyword::krparen;
        case '-':
            return eatc('=') ? Keyword::ksubeq : eatc('>') ? Keyword::karrow : Keyword::ksub;
        case '+':
            return eatc('=') ? Keyword::kaddeq : Keyword::kadd;
        case '[':
            return eatc('[') ? Keyword::klsquare2 : Keyword::klsquare;
        case ']':
            return eatc(']') ? Keyword::krsquare2 : Keyword::krsquare;
        case '{':
            return Keyword::klbrace;
        case '}':
            return Keyword::krbrace;
        case '|':
            return eatc('|') ? Keyword::kor : eatc('=') ? Keyword::kbitoreq : Keyword::kbitor;
        case ':':
            return eatc('=') ? Keyword::kassign : Keyword::kcolon;
        case '<':
            if(eatc('<'))
            {
                if(eatc('='))
                    return Keyword::kshleq;
                return Keyword::kshl;
            }

            return eatc('=') ? Keyword::klte : Keyword::klt;
        case '>':
            if(eatc('>'))
            {
                if(eatc('='))
                    return Keyword::kshreq;
                return Keyword::kshr;
            }

            return eatc('=') ? Keyword::kgte : Keyword::kgt;
        case ',':
            return Keyword::kcomma;
        case '.':
            return Keyword::kdot;
        case '=':
            // `==` is valid but `=` is not
            if(eatc('=')) return Keyword::keq;
            if(eatc('>')) return Keyword::kbigarrow;
            // fallthrough
        default:
            // invalid symbol
            return Keyword::kinvalid;
            break;
        }
    }

    std::pair<std::string, Keyword> keys[] = {
#define KEYWORD(id, str) { str, Keyword::id },
#include "keywords.inc"
    };

    Token Lexer::alpha(int c)
    {
        std::string buffer;
        buffer += c;

        for(;;)
        {
            c = peekc();
            if(!(isalnum(c) || c == '_'))
                break;

            buffer += nextc();
        }

        for(auto& [name, key] : keys)
            if(name == buffer)
                return Key(key);

        return Ident(buffer);
    }

    int Lexer::nextc()
    {
        int i = in->get();

        return i;
    }
    
    int Lexer::peekc()
    {
        return in->peek();
    }

    bool Lexer::eatc(int c)
    {
        if(peekc() == c)
        {
            nextc();
            return true;
        }

        return false;
    }

    Token Lexer::hex()
    {
        std::string buffer;
        while(isxdigit(peekc()))
            buffer += nextc();

        if(buffer.empty())
            return Invalid("empty hex number");

        return Int(std::stoull(buffer, nullptr, 16));
    }

    Token Lexer::binary()
    {
        std::string buffer;
        while(peekc() == '0' || peekc() == '1')
            buffer += nextc();

        if(buffer.empty())
            return Invalid("empty binary number");

        return Int(std::stoull(buffer, nullptr, 2));
        
    }

    Token Lexer::number(int c)
    {
        if(c == '0')
        {
            switch(nextc())
            {
                // hex format
            case 'x':
                return hex();
                // binary format
            case 'b':
                return binary();
            default:
                return Invalid("invalid number encoding (numbers cannot start with 0)");
            }
        }

        bool isfloat = false;
        std::string buffer;

        buffer += c;

        while(peekc() == '.' || isdigit(peekc()))
        {
            int i = nextc();
            if(i == '.')
            {
                if(isfloat)
                    return Invalid("invalid number encoding (floats cannot have multiple decimal places)");

                isfloat = true;
            }
            buffer += i;
        }

        if(isfloat)
            return Float(std::stod(buffer));
        else
            return Int(std::stoull(buffer));
    }

    char Lexer::getchar(bool* b)
    {
        int i = nextc();
        if(i == '"')
        {
            *b = false;
            return '\0';
        }
        
        if(i == '\\')
        {
            switch(nextc())
            {
            case 'n': return '\n';
            case 't': return '\t';
            case 'v': return '\v';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            default:
                return i;
                // TODO: invalid escape
            }
        }

        return i;
    }

    String Lexer::str()
    {
        std::string buffer = "";

        bool b = true;
        while(b)
            buffer += getchar(&b);

        return String(buffer);
    }

    Char Lexer::ch()
    {
        bool stub;
        Char c(getchar(&stub));

        return c;
    }

    int Lexer::skip_whitespace(int i)
    {
        while(isspace(i))
            i = nextc();

        return i;
    }

    Token Lexer::parse()
    {
        int i = nextc();

        i = skip_whitespace(i);

        while(i == '#')
        {
            while(i != '\n')
                i = nextc();

            i = skip_whitespace(i);
        }

        if(isalpha(i) || i == '_')
        {
            return alpha(i);
        }
        else if(isdigit(i))
        {
            return number(i);
        }
        else if(i == '"')
        {
            return str();
        }
        else if(i == '\'')
        {
            return ch();
        }
        else if(i == EOF)
        {
            return Eof();
        }
        else 
        {
            return symbol(i);
        }
    }
}