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
        , depth(0)
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
            str += next(true);

            if (str.ends_with(limit)) {
                break;
            }
        }

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
            throw LexerError(this, LexerError::END);
            // TODO: issue diagnostic
        }
    }

    Token Lexer::token(const Range& start, Token::Type type, TokenData data) {
        auto range = start.to(here);
        return Token(range, type, data);
    }

    Key Lexer::symbol(c32 c) {
        switch (c) {
        case '(': return Key::LPAREN;
        case ')': return Key::RPAREN;
        case '[': return Key::LSQUARE;
        case ']': return Key::RSQUARE;
        case '{': return Key::LBRACE;
        case '}': return Key::RBRACE;
        case '@': return Key::AT;
        case '?': return Key::QUESTION;
        case ':': return eat(':') ? Key::COLON2 : Key::COLON;
        case '.': {
            if (eat('.')) {
                if (eat('.')) {
                    return Key::DOT3;
                }
                return Key::DOT2;
            }
            return Key::DOT;
        }
        case ',': return Key::COMMA;
        case '!': {
            if (eat('<')) {
                depth++;
                return Key::BEGIN;
            } else if (eat('=')) {
                return Key::NEQ;
            } else {
                return Key::NOT;
            }
        }
        case '<': {
            if (eat('<')) {
                return eat('=') ? Key::SHLEQ : Key::SHL;
            } else {
                return eat('=') ? Key::LTE : Key::LT;
            }
        }
        case '>': {
            if (depth > 0) {
                depth--;
                return Key::END;
            } else if (eat('>')) {
                return eat('=') ? Key::SHREQ : Key::SHR;
            } else {
                return eat('=') ? Key::GTE : Key::GT;
            }
        }
        
        case '+': return eat('=') ? Key::ADDEQ : Key::ADD;

        default:
            throw LexerError(this, LexerError::CHAR);
        }
    }

#define PARSE_NUM(v, i, check) 

    c32 Lexer::encodeChar() {
        c32 out = 0;

        for (;;) {
            c32 c = next(true);
            if (c == '\'') {
                break;
            } else if (c == '\\') {
                c = next();
                switch (c) {
                case 'a': out = out * 10 + '\a'; break;
                case 'b': out = out * 10 + '\b'; break;
                case 'f': out = out * 10 + '\f'; break;
                case 'n': out = out * 10 + '\n'; break;
                case 'r': out = out * 10 + '\r'; break;
                case 't': out = out * 10 + '\t'; break;
                case 'v': out = out * 10 + '\v'; break;
                case '"': out = out * 10 + '\"'; break;
                case '\'': out = out * 10 + '\''; break;
                case '\\': out = out * 10 + '\\'; break;
                case 'x': {
                    fprintf(stderr, "hex\n");
                    out = out * 16 + next() - '0';
                    while (isxdigit(peek())) {
                        fprintf(stderr, "peek: %d %d\n", out, peek() - '0');
                        out = out * 16 + next() - '0';
                    }
                    fprintf(stderr, "temp %d\n", out);
                    break;
                }
                case 'd': {
                    out = out * 10 + next() - '0';
                    while (isdigit(peek())) {
                        out = out * 10 + next() - '0';
                    }
                    break;
                }
                default: 
                    out = out * 10 + c;
                    warn(here, "invalid character escape in char"); 
                    break;
                }
            } else {
                out = out * 10 + c;
            }
        }

        return out;
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
        } else if (c == '\'') {
            c32 c = encodeChar();
            return token(start, Token::CHAR, { .letter = c });
        } else {
            Key key = symbol(c);
            return token(start, Token::KEY, { .key = key });
        }
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

    const utf8::string& Lexer::file() const {
        return name;
    }
}
