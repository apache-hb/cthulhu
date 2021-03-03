#include "lexer.hpp"
#include "error.hpp"

#include <fmt/core.h>
#include <unordered_map>

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
    Diagnostic::Diagnostic(Range range, std::string message)
        : range(range)
        , message(message)
    { }

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
            c = next();
            switch (c) {
            case 'a': out->push_back('\a'); break;
            case 'b': out->push_back('\b'); break;
            case 'f': out->push_back('\f'); break;
            case 'n': out->push_back('\n'); break;
            case 'r': out->push_back('\r'); break;
            case 't': out->push_back('\t'); break;
            case 'v': out->push_back('\v'); break;
            case '"': out->push_back('\"'); break;
            case '\'': out->push_back('\''); break;
            case '\\': out->push_back('\\'); break;
            case 'x': encodeInt(out, BASE16); break;
            case 'd': encodeInt(out, BASE10); break;
            default: 
                out->push_back(c);
                warn(here, "invalid character escape");
                break;
            }
        } else {
            out->push_back(c);
        }

        return true;
    }

    void Lexer::encodeInt(utf8::string* out, Base base) {
        if (base == BASE10) {
            while (isdigit(peek())) {
                uint8_t num = next() - '0';
                if (isdigit(peek())) {
                    num += next() - '0';
                }
                out->push_back(num);
            }
        } else if (base == BASE16) {
            while (isxdigit(peek())) {
                uint8_t num = next() - '0';
                if (isxdigit(peek())) {
                    num += next() - '0';
                }
                out->push_back(num);
            }
        } else {
            // TODO: issue diagnostic
        }
    }

    Token Lexer::token(const Range& start, Token::Type type, TokenData data) {
        auto range = start.to(here);
        return Token(range, type, data);
    }

    void Lexer::warn(const Range& range, const std::string& message) {
        messages.emplace(range, message);
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

    std::optional<Diagnostic> Lexer::diagnostic() {
        if (messages.empty()) {
            return std::nullopt;
        }

        auto last = messages.back();
        messages.pop();
        return last;
    }

    utf8::string Lexer::format(const Diagnostic& diag) const {
        return fmt::format("[{}:{}]: {}\n--> {}", diag.range.line, diag.range.column, name.c_str(), diag.message);
    }
}
