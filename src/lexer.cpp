#include <istream>
#include <variant>
#include <map>
#include <string>
#include <array>
#include <type_traits>

#include "hash.h"

namespace ct {
    bool isident1(int c) { return isalpha(c) || c == '_'; }
    bool isident2(int c) { return isalnum(c) || c == '_'; }

    enum class CollectResult {
        keep,
        skip,
        end
    };

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
        HASH,
        COMMA,
        DOT,
        QUESTION
    };

    struct Token {
        enum { eof, ident, string, key, integer, number, character } type;
        using type_t = decltype(type);

        std::variant<unsigned long long, std::string, Keyword, double> data;

        Token(type_t t)
            : type(t)
            , data(0ULL)
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
            } else if (c == '"') {
                // string
            } else if (c == '\'') {
                // character
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
            default: return Token(Token::ident, std::move(buf));
            }
        }

        Token parse_hex() {
            auto hex = collect([](char c) {
                if (isxdigit(c)) 
                    return CollectResult::keep;
                else if (c == '_')
                    return CollectResult::skip;
                else
                    return CollectResult::end;
            });

            auto val = std::stoull(hex, nullptr, 16);

            return Token(Token::integer, val);
        }

        Token parse_bin() {
            auto bin = collect([](char c) {
                if (c == '0' || c == '1')
                    return CollectResult::keep;
                else if (c == '_')
                    return CollectResult::skip;
                else
                    return CollectResult::end;
            });

            auto val = std::stoull(bin, nullptr, 2);

            return Token(Token::integer, val);
        }

        Token parse_oct() {
            auto oct = collect([](char c) {
                if ('0' <= c && c >= '7') 
                    return CollectResult::keep;
                else if (c == '_')
                    return CollectResult::skip;
                else
                    return CollectResult::end;
            });

            auto val = std::stoull(oct, nullptr, 8);

            return Token(Token::integer, val);
        }

        Token number0() {
            if (consume_any('x', 'X')) {
                return parse_hex();
            } else if (consume_any('b', 'B')) {
                return parse_bin();
            } else if (consume_any('o', 'O')) {
                return parse_oct();
            } else {
                printf("TODO\n");
                std::exit(2);
                // oh no
            }
        }

        Token parse_number(char c) {
            bool decimal = false;
            auto num = collect(c, [&decimal](char c) {
                if (c == '.') {
                    decimal = true;
                    return CollectResult::keep;
                } else if (isdigit(c)) {
                    return CollectResult::keep;
                } else if (c == '_') {
                    return CollectResult::skip;
                } else {
                    return CollectResult::end;
                }
            });

            if (decimal) {
                return Token(Token::number, std::stod(num));
            } else {
                return Token(Token::integer, std::stoull(num));
            }
        }

        Token number(char c) {
            if (c == '0') {
                return number0();
            } else {
                return parse_number(c);
            }
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
            case '#': return Token(Token::key, Keyword::HASH);
            default:
                // TODO: handle this
                printf("TODO 2\n");
                std::exit(1);
                break;
                // oh no
            }
        }

        template<typename F>
        std::string collect(char c, F&& filter) {
            std::string out = {c};
            
            using res_t = std::result_of_t<F(char)>;
            if constexpr (std::is_same_v<bool, res_t>) {
                while (filter(peek()))
                    out += read();
            } else {
                while (true) { 
                    auto res = filter(peek());
                    if (res == CollectResult::keep) {
                        out += read();
                    } else if (res == CollectResult::skip) {
                        read();
                    } else {
                        break;
                    }
                }
            }

            return out;
        }

        template<typename F>
        std::string collect(F&& filter) {
            std::string out;
            
            while (true) { 
                auto res = filter(peek());
                if (res == CollectResult::keep) {
                    out += read();
                } else if (res == CollectResult::skip) {
                    read();
                } else {
                    break;
                }
            }

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

        template<typename... T>
        bool consume_any(T... chars) {
            for (char c : {chars...}) 
                if (consume(c))
                    return true;
            
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