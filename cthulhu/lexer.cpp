#include "lexer.hpp"
#include "error.hpp"

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

    bool isident1(c32 c) {
        return isalpha(c) || c == '_';
    }

    bool isident2(c32 c) {
        return isalnum(c) || c == '_';
    }

    const std::unordered_map<utf8::string, cthulhu::Key> keywords = {
#define OP(id, str) { str, cthulhu::Key::id },
#define KEY(id, str) { str, cthulhu::Key::id },
#include "keys.inc"
    };
}

namespace cthulhu {
    Lexer::Lexer(Stream stream, const utf8::string& name)
        : stream(stream)
        , here(this)
        , name(name)
    { }

    c32 Lexer::next(bool end) {
        c32 c = stream.next();

        if (c == END) {
            if (end) {
                throw LexerError(this, LexerError::END);
            }

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

    Token Lexer::ident(const Range& start, c32 first) {
        if (first == 'R' && eat('"')) {
            // this is the start of a raw string literal
            auto* str = rstring();
            return token(start, Token::STRING, { .string = str });
        }

        auto ident = collect(first, isident2);

        // search the keyword map for the identifier
        if (auto search = keywords.find(ident); search != keywords.end()) {
            // if we found something then this is a keyword
            return token(start, Token::KEY, { .key = search->second });
        } else {
            // otherwise its an identifier
            return token(start, Token::IDENT, { .ident = idents.intern(ident) });
        }
    }

    const utf8::string* Lexer::rstring() {
        utf8::string str;
        
        // collect the prefix that this raw string will be limited by
        utf8::string limit = ")" + collect(END, [](c32 c) { return c != '('; }) + '"';
        
        // discard the leading `(`
        next();

        while (true) {
            c32 c = next(true);

            printf("%d ", (int)c);
            str += c;

            if (str.ends_with(limit)) {
                break;
            }
        }

        printf("\n%ld\n", limit.length());

        // return the string with the trailing characters sliced off
        auto out = str.substr(0, str.length() - limit.length());
        return strings.intern(out);
    }

    const utf8::string* Lexer::string() {
        utf8::string out;

        while (encode(&out));

        return strings.intern(out);
    }

    bool Lexer::encode(utf8::string* out) {
        c32 c = next();
    
        if (c == '"') {
            // check for the end of the string
            return false;
        }

        // string escapes
        if (c == '\\') {

        } else {
            out->push_back(c);
        }

        return true;
    }

    Token Lexer::token(const Range& start, Token::Type type, TokenData data) {
        auto range = start.to(here);
        return Token(range, type, data);
    }

    Token Lexer::read() {
        c32 c = skip();
        auto start = here;
        
        if (c == END) {
            return token(start, Token::END, {});
        } else if (isident1(c)) {
            return ident(start, c);
        } else if (c == '"') {
            auto* str = string();
            return token(start, Token::STRING, { .string = str });
        }

        throw LexerError(this, LexerError::CHAR);
    }
}
