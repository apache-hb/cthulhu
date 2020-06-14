#include <istream>
#include <variant>
#include <map>
#include <string>

#include <hash.h>

namespace ct {
    bool isident1(int c) { return isalpha(c) || c == '_'; }
    bool isident2(int c) { return isalnum(c) || c == '_'; }

    enum class Keyword {
        IMPORT,
        TYPE
    };

    struct Token {
        enum { eof, ident, string, key } type;
        using type_t = decltype(type);

        std::variant<uint64_t> data;

        Token(type_t t)
            : type(t)
        { }

        template<typename T>
        Token(type_t t, T&& d)
            : type(t)
            , data(d)
        { }
    };

    struct Lexer {
        Token next() {
            auto c = skip([](auto c) { return isspace(c); });

            if (c == EOF) {
                return Token(Token::eof);
            } else if (isident1(c)) {
                return ident(c);
            } else if (isdigit(c)) {
                return number(c);
            } else {
                return symbol(c);
            }
        }

    private:
        Token ident(char c) {
            std::string buf = collect(c, isident2);

            switch (crc32(buf)) {
            case crc32("import"): return Token(Token::key, Keyword::IMPORT);
            case crc32("type"): return Token(Token::key, Keyword::TYPE);
            default:
                return Token(Token::ident, std::move(buf));
            }
        }

        Token number(char c) {

        }

        Token symbol(char c) {
            switch (c) {
            case '+':
            case '-':
            case '/':
            case '*':
            case '%':
            case '&':
            case '|':
            case '[':
            case ']':
            case '{':
            case '}':
            case '(':
            case ')':
            case '!':
            case '=':
            case '@':
            case ',':
            case '.':
            case '?':
            case '<':
            case '>':
            case '^':
            default:
                // oh no
            }
        }

        template<typename F>
        std::string collect(char c, F&& filter) {
            std::string out = {c};
            while (filter(peek()))
                out += read();

            return out;
        }

        std::istream source;
        int ahead;

        int read() {
            int temp = ahead;
            ahead = source.get();
            return temp;
        }

        int peek() {
            return ahead;
        }

        bool consume(int c) {
            if (peek() == c) {
                read();
                return true;
            }
            return false;
        }

        template<typename F>
        int skip(F&& func) {
            int c = read();
            while (func(c))
                c = read();

            return c;
        }
    };
}