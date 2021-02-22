#include "cthulhu.hpp"

#include <unordered_map>

namespace cthulhu {
    bool whitespace(char32_t c) {
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

    bool newline(char32_t c) {
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

    bool isident1(char32_t c) {
        return isalpha(c) || c == '_';
    }

    bool isident2(char32_t c) {
        return isalnum(c) || c == '_';
    }

    ///
    /// stream handling
    ///
    Stream::Stream(StreamHandle* handle)
        : handle(handle)
        , lookahead(handle->next())
    { }

    char32_t Stream::next() {
        char32_t temp = lookahead;
        lookahead = handle->next();
        return temp;
    }

    char32_t Stream::peek() {
        return lookahead;
    }



    ///
    /// lexing
    ///

    Lexer::Lexer(Stream stream)
        : stream(stream)
        , here(Location(this, 0, 0, 0))
        , text(u8"")
    { }

    utf8::string Lexer::slice(Range* range) {
        return text.substr(range->offset, range->length);
    }

    char32_t Lexer::skip() {
        char32_t c = next();

        while (whitespace(c)) {
            c = next();
        }

        return c;
    }

    char32_t Lexer::next() {
        char32_t c = stream.next();
        text += c;

        if (newline(c)) {
            here.line++;
            here.column = 0;
        } else {
            here.column++;
        }

        here.offset++;

        return c;
    }

    char32_t Lexer::peek() {
        return stream.peek();
    }

    bool Lexer::eat(char32_t c) {
        if (peek() == c) {
            next();
            return true;
        }

        return false;
    }

    utf8::string Lexer::collect(char32_t start, bool(*filter)(char32_t)) {
        utf8::string out = start == END ? u8"" : utf8::string(start);

        while (filter(peek())) {
            out += next();
        }

        return out;
    }

    using KeyMap = unordered_map<utf8::string, Key::Word>;

#define KEY(id, str) { u8 ## str, Key::id },

    const KeyMap keys = {
#include "keys.inc"
    };

    Token* Lexer::read() {
        char32_t c = skip();
        Location start = here;
        Token* token = nullptr;

        if (c == END) {
            token = new End();
        } else if (isident1(c)) {
            utf8::string str = collect(c, [](char32_t c) { return isident2(c); });

            if (auto iter = keys.find(str); iter == keys.end()) {
                token = new Ident(str);
            } else {
                token = new Key(iter->second);
            }

        } else if (isdigit(c)) {
            utf8::string str;
            uint64_t num;

            if (c == '0' && eat('x')) {
                str = collect(END, [](char32_t c) -> bool { return isxdigit(c); });
                num = strtoull(str.c_str(), nullptr, 16);
            } else if (c == '0' && eat('b')) {
                str = collect(END, [](char32_t c) { return c == '0' || c == '1'; });
                num = strtoull(str.c_str(), nullptr, 2);
            } else {
                str = collect(c, [](char32_t c) -> bool { return isdigit(c); });
                num = strtoull(str.c_str(), nullptr, 10);
            }

            if (isident1(peek())) {
                utf8::string suffix = collect(next(), [](char32_t c) { return isident2(c); });
                token = new Int(num, suffix);
            } else {
                token = new Int(num);
            }
        } else if (c == '"') {
            utf8::string str;
            while (true) {
                c = next();
                if (c == '"') {
                    break;
                } else if (c == '\\') {
                    str += next();
                } else {
                    str += c;
                }
            }
            token = new String(str);
        } else {
            // symbol
            token = nullptr;
        }

        token->range = start.to(here);
        return token;
    }


    ///
    /// token data
    ///

    Location::Location(Lexer* lexer, size_t offset, size_t line, size_t column) 
        : lexer(lexer) 
        , offset(offset) 
        , line(line) 
        , column(column) 
    { }

    Range* Location::to(Location other) {
        return new Range(lexer, offset, line, column, other.offset - offset);
    }

    Range::Range(Lexer* lexer, size_t offset, size_t line, size_t column, size_t length)
        : Location(lexer, offset, line, column)
        , length(length)
    { }



    Ident::Ident(utf8::string id) 
        : ident(id) 
    { }

    bool Ident::operator==(const Ident& other) const {
        return ident == other.ident;
    }



    Key::Key(Key::Word word) 
        : key(word)
    { }

    bool Key::operator==(const Key& other) const {
        return key == other.key;
    }



    Int::Int(size_t num, utf8::string str)
        : num(num)
        , suffix(str)
    { }

    bool Int::operator==(const Int& other) const {
        return num == other.num && suffix == other.suffix;
    }



    String::String(utf8::string str)
        : str(str)
    { }

    bool String::operator==(const String& other) const {
        return str == other.str;
    }
}