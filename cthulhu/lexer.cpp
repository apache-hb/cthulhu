#include "lexer.hpp"

namespace {
    bool whitespace(c32 c) {
        switch (c) {
        case 0x0009:
        case 0x000A:
        case 0x000B:
        case 0x000C:
        case 0x000D:
        case 0x001C:
        case 0x001D:
        case 0x001E:
        case 0x001F:
        case 0x0020:
        case 0x0085:
        case 0x00A0:
        case 0x1680:
        case 0x2000:
        case 0x2001:
        case 0x2002:
        case 0x2003:
        case 0x2004:
        case 0x2005:
        case 0x2006:
        case 0x2007:
        case 0x2008:
        case 0x2009:
        case 0x200A:
        case 0x2028:
        case 0x2029:
        case 0x202F:
        case 0x205F:
        case 0x3000:
            return true;

        default:
            return false;
        }
    }

    bool newline(c32 c) {
        switch (c) {
        case 0x000A:
        case 0x000B:
        case 0x000C:
        case 0x000D:
        case 0x001C:
        case 0x001D:
        case 0x001E:
        case 0x0085:
        case 0x2028:
        case 0x2029:
            return true;

        default:
            return false;
        }
    }
}

namespace cthulhu {
    Lexer::Lexer(Stream stream, utf8::string name)
        : stream(stream)
        , here(this)
        , name(name)
    { }

    c32 Lexer::next() {
        c32 c = stream.next();

        if (c == END) {
            return END;
        }

        text += c;

        return c;
    }
    
    c32 Lexer::peek() {
        return stream.peek();
    }
    
    c32 Lexer::skip() {
        c32 c = next();

        while (true) {
            if (c == '#') {
                while (!newline(c)) {
                    c = next();
                }
            } else if (!whitespace(c)) {
                break;
            } else {
                c = next();
            }
        }

        return c;
    }
    
    bool Lexer::eat(c32 c) {
        if (peek() == c) {
            next();
            return true;
        }

        return false;
    }
    


    Token Lexer::read() {
        c32 c = skip();
        auto start = here;
        
        if (c == END) {
            
        }

        return Token();
    }
}
