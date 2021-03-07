#pragma once

#include "stream.hpp"
#include "token.hpp"
#include "pool.hpp"
#include "fwd.hpp"

#include <string>
#include <optional>
#include <queue>

namespace cthulhu {
    struct Diagnostic {
        Diagnostic(Range range, std::string message);

        Range range;
        std::string message;
    };

    struct Lexer {
        Lexer(Stream stream, const utf8::string& name, ptr<StringPool> idents = nullptr, ptr<StringPool> strings = nullptr);

        Token read();

        // get a diagnostic
        std::optional<Diagnostic> diagnostic();

        // format a diagnostic into a pretty string
        utf8::string format(const Diagnostic& diag) const;

        // get file name
        const utf8::string& file() const;
    
    protected:
        // get the next char from the strea
        // if `end` is true then throw an exception
        // if the EOF is reached
        c32 next(bool end = false);

        // peek the next char in the stream without moving the
        // read head
        c32 peek();
        
        // skip whitespace and comments until the next
        // non whitespace character is found
        c32 skip();

        // if a `c` is the next character in the stream
        // then consume it and return true,
        // otherwise do nothing and return false
        bool eat(c32 c);

        // collect characters into a string and break when func returns false
        template<typename F>
        utf8::string collect(c32 first, F&& func) {
            utf8::string out = first == END ? "" : utf8::string(first);
            while (func(peek())) {
                out += next();
            }
            return out;
        }

        template<typename F>
        size_t collectNumber(size_t base, c32 first, F&& ok, c32(*func)(c32)) {
            size_t out = first != END ? first - '0' : 0;
            
            while (true) {
                c32 c = peek();
                if (c == '_') {
                    next();
                } else if (ok(c)) {
                    out = (out * base) + func(next());
                } else {
                    break;
                }
            }

            return out;
        }

        // returns either a raw string, an ident, or a keyword
        Token ident(const Range& start, c32 first);

        // collect a raw string
        const utf8::string* rstring();

        // collect a single line string
        const utf8::string* string();

        // lex a number and suffix
        Number digit(c32 c);

        // decode a character in a string and place it 
        // in `out`, return true if the string lexing should continue
        // false if the end of the string has been reached
        bool encode(utf8::string* out);

        // integer encoding
        enum Base {
            BASE2, // 0b101010
            BASE10, // 1234567890
            BASE16 // 0x1234567890abcdef
        };

        // encode an integer escape into a string
        void encodeInt(utf8::string* out, Base base);

        // create a token and set its range properly
        Token token(const Range& start, Token::Type type, TokenData data);

        // add a warning to the message queue
        void warn(const Range& range, const std::string& message);

        // read char literal
        uint32_t encodeChar();

        // lex a symbol/keyword
        Key symbol(c32 c);

        // out source stream
        Stream stream;

        // our current position
        Range here;

        // name of the lexer
        const utf8::string& name;

        // all currently read text
        utf8::string text;

        // all idents that have been lexed
        ptr<StringPool> idents;

        // all strings that have been lexed
        ptr<StringPool> strings;

        // lexing diagnostic messages
        std::queue<Diagnostic> messages;

        // template depth
        int depth;
    };
}