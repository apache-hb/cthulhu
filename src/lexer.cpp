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
        TYPE,
        DEF,

        ADD,
        ADDEQ,

        SUB,
        SUBEQ,

        MUL,
        MULEQ,

        DIV,
        DIVEQ,

        MOD,
        MODEQ,

        BITAND,
        BITANDEQ,

        BITOR,
        BITOREQ,

        BITXOR,
        BITXOREQ,

        SHL,
        SHLEQ,

        SHR,
        SHREQ,

        AND,
        OR,

        LSQUARE,
        RSQUARE,

        LPAREN,
        RPAREN,

        LBRACE,
        RBRACE,

        GT,
        GTE,

        LT,
        LTE,

        EQ,
        NEQ,

        NOT,
        ASSIGN,

        AT,
        COMMA,
        DOT,
        QUESTION
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
            case crc32("def"): return Token(Token::key, Keyword::DEF);
            default:
                return Token(Token::ident, std::move(buf));
            }
        }

        Token number(char c) {

        }

        Token symbol(char c) {
            switch (c) {
            case '+': return Token(Token::key, consume('=') ? Keyword::ADDEQ : Keyword::ADD);
            case '-': return Token(Token::key, consume('=') ? Keyword::SUBEQ : Keyword::SUB);
            case '/': return Token(Token::key, consume('=') ? Keyword::DIVEQ : Keyword::DIV);
            case '*': return Token(Token::key, consume('=') ? Keyword::MULEQ : Keyword::MUL);
            case '%': return Token(Token::key, consume('=') ? Keyword::MODEQ : Keyword::MOD);
            case '^': return Token(Token::key, consume('=') ? Keyword::BITXOREQ : Keyword::BITXOR);
            case '&': return Token(Token::key, consume('&') ? Keyword::AND : consume('=') ? Keyword::BITANDEQ : Keyword::BITAND);
            case '|': return Token(Token::key, consume('|') ? Keyword::OR : consume('=') ? Keyword::BITOREQ : Keyword::BITOR);
            case '[': return Token(Token::key, Keyword::LSQUARE);
            case ']': return Token(Token::key, Keyword::RSQUARE);
            case '{': return Token(Token::key, Keyword::LBRACE);
            case '}': return Token(Token::key, Keyword::RBRACE);
            case '(': return Token(Token::key, Keyword::LPAREN);
            case ')': return Token(Token::key, Keyword::RPAREN);
            case '!': return Token(Token::key, consume('=') ? Keyword::NEQ : Keyword::NOT);
            case '=': return Token(Token::key, consume('=') ? Keyword::EQ : Keyword::ASSIGN);
            case '@': return Token(Token::key, Keyword::AT);
            case ',': return Token(Token::key, Keyword::COMMA);
            case '.': return Token(Token::key, Keyword::DOT);
            case '?': return Token(Token::key, Keyword::QUESTION);
            case '<': return Token(Token::key, consume('<') ? consume('=') ? Keyword::SHLEQ : Keyword::SHL : consume('=') ? Keyword::GTE : Keyword::GT);
            case '>': return Token(Token::key, consume('>') ? consume('=') ? Keyword::SHREQ : Keyword::SHR : consume('=') ? Keyword::LTE : Keyword::LT);
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