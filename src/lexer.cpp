#include <istream>
#include <variant>
#include <map>
#include <string>
#include <array>
#include <optional>
#include <type_traits>
#include <sstream>
#include <string.h>
#include <iostream>

#include "hash.h"

namespace ct {
    bool isident1(int c) { return isalpha(c) || c == '_'; }
    bool isident2(int c) { return isalnum(c) || c == '_'; }
    bool isnumber(int c) { return isdigit(c) || c == '.'; }

    enum class CollectResult {
        keep,
        skip,
        end
    };

    enum class Keyword {
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keywords.inc"
        INVALID
    };

    const char* kstr(Keyword key) {
        switch (key) {
#define KEY(id, str) case Keyword::id: return str;
#define OP(id, str) case Keyword::id: return str;
#include "keywords.inc"
        default: return "Invalid";
        }
    }

    struct SourcePos {
        std::string_view name;
        std::istream *source;
        uint64_t dist;
        int col;
        int line;

        std::string full_line() const {
            printf("aaa");
            auto h = source->tellg();
            std::cout << h;

            std::string buf;

            source->seekg(dist - col - 1);

            while (true) {
                int c = source->get();
                if (c == '\n')
                    break;

                buf += c;
            }

            source->seekg(h);

            return buf;
        }
    };

    using u64 = unsigned long long;

    struct Token {
        enum {
            ident, // std::string
            string, // std::string
            key, // Keyword
            integer, // u64
            character, // u64
            eof, // std::monostate
            invalid // std::monostate
        } type;
        using type_t = decltype(type);

        SourcePos pos;

        // TODO: figure out a not stupid way of handling strings and stuff
        // probably just make everything utf-8 because that seems easy enough
        std::variant<u64, std::string, Keyword, std::monostate> data;

        Token(type_t t)
            : type(t)
            , data(std::monostate())
        { }

        template<typename T>
        Token(type_t t, T&& d)
            : type(t)
            , data(d)
        { }

        bool is(type_t t) const {
            return type == t;
        }

        template<typename T>
        bool is(type_t t, T val) const {
            if (is(t)) {
                return std::get<T>(data) == val;
            }

            return false;
        }

        std::string str() const {
            using namespace std::string_literals;
            switch (type) {
            case ident: return "Ident(" + std::get<std::string>(data) + ")";
            case key: return "Key('"s + ct::kstr(std::get<Keyword>(data)) + "')";
            case eof: return "EOF";
            case integer: return "Int(" + std::to_string(std::get<u64>(data)) + ")";
            case string: return "String(\"" + std::get<std::string>(data) + "\")";
            default: return "Error";
            }
        }

        size_t len() const {
            switch (type) {
            case string:
                // TODO: hack
                return std::get<std::string>(data).size() + 2;
            case ident:
                return std::get<std::string>(data).size();
            case key:
                return strlen(kstr(std::get<Keyword>(data)));
            default: return 0;
            }
        }

        std::string underline(std::string_view msg = "") const {
            std::ostringstream os;
            printf("aaaa");

            os  << msg << "\n"
                << " --> " << pos.name << ":" << pos.line << ":" << pos.col << "\n"
                << " |\n"
                << " |" << pos.full_line() << "\n"
                << " |" << std::string(pos.col - 1, ' ') << std::string(len(), '^') << "\n";

            return os.str();
        }
    };

    struct Lexer {
        Lexer(std::istream *in, std::string_view name = "<unnamed>")
            : source(in)
        {
            ahead = source->get();
            pos = { name, source, 1, 0, 0 };

            if (ahead == '\n') {
                pos.col = 0;
                pos.line = 1;
            } else {
                pos.col = 1;
                pos.line = 0;
            }
        }

        using Tok = Token;

        Tok next() {
            auto c = skip_comments();

            Tok out = Tok(Tok::invalid);
            auto here = pos;

            if (c == EOF) {
                out = Tok(Tok::eof);
            } else if (c == '"') {
                out = plain_string();
            } else if (c == '\'') {
                out =  char_token();
            } else if (c == 'R') {
                if (consume('"')) {
                    out = raw_string();
                } else {
                    out = ident('R');
                }
            } else if (isident1(c)) {
                out = ident(c);
            } else if (isdigit(c)) {
                out = number(c);
            } else {
                out = symbol(c);
            }

            out.pos = here;
            return out;
        }

    private:
        Tok raw_string() {
            if (!consume('(')) {
                return Tok(Tok::string, std::string(nullptr));
            }

            std::string buf;
            while (true) {
                auto c = read();
                if (c == ')' && consume('"'))
                    break;

                buf += c;
            }

            return Tok(Tok::string, buf);
        }

        char oct_escape() {
            printf("TODO: oct escape");
            return 0;
        }

        char hex_escape() {
            printf("TODO: hex escape\n");
            return 0;
        }

        char unicode_escape() {
            printf("TODO: unicode escape\n");
            return 0;
        }

        std::optional<char> read_char() {
            auto c = read();
            if (c == '\n') {
                // newlines in strings are not a thing we allow
                printf("no newlines in strings\n");
                return '\n';
            }

            if (c == '"') {
                return std::nullopt;
            }

            if (c == '\\') {
                c = read();
                switch (c) {
                case 'a': return '\a';
                case 'b': return '\b';
                case 'f': return '\f';
                case 'n': return '\n';
                case 'r': return '\r';
                case 't': return '\t';
                // what even is a vertical tab?
                case 'v': return '\v';
                case '\'': return '\'';
                case '"': return '"';
                // octal escape
                case 'o':
                    return oct_escape();
                // hex escape
                case 'x':
                    return hex_escape();
                // unicode escape
                case 'u':
                    return unicode_escape();
                // invalid escape
                default:
                    printf("invalid escape char %c\n", c);
                    return c;
                }
            }

            return c;
        }

        Tok plain_string() {
            std::string buf;

            while (true) {
                auto c = read_char();
                if (!c.has_value())
                    break;

                buf += c.value();
            }

            return Tok(Tok::string, buf);
        }

        Tok char_token() {
            auto c = read_char();
            if (!consume('\'')) {
                // error
                printf("closing ' missing \n");
                std::exit(500);
            }
            return Tok(Tok::character, c.value());
        }

        int skip_comments() {
            int c = skip([](auto c) {
                return isspace(c);
            });

            while (c == '/') {
                if (consume('*')) {
                    // multiline comment
                    int depth = 1;

                    while (depth != 0) {
                        c = read();
                        if (c == '/' && consume('*')) {
                            depth++;
                        } else if (c == '*' && consume('/')) {
                            depth--;
                        }
                    }

                    c = read();

                    while (isspace(c))
                        c = read();
                } else if (consume('/')) {
                    // single line comment
                    while (peek() != '\n')
                        c = read();

                    // then skip more whitespace
                    while (isspace(c))
                        c = read();
                } else {
                    // not a comment
                    break;
                }
            }

            return c;
        }

        Tok ident(char c) {
            std::string buf = collect(c, isident2);

            switch (crc32(buf)) {
#define KEY(id, str) case crc32(str): return Tok(Tok::key, Keyword::id);
#include "keywords.inc"
            default: return Tok(Tok::ident, std::move(buf));
            }
        }

        Tok parse_hex() {
            auto hex = collect([](char c) {
                if (isxdigit(c))
                    return CollectResult::keep;
                else if (c == '_')
                    return CollectResult::skip;
                else
                    return CollectResult::end;
            });

            auto val = std::stoull(hex, nullptr, 16);

            return Tok(Tok::integer, val);
        }

        Tok parse_bin() {
            auto bin = collect([](char c) {
                if (c == '0' || c == '1')
                    return CollectResult::keep;
                else if (c == '_')
                    return CollectResult::skip;
                else
                    return CollectResult::end;
            });

            auto val = std::stoull(bin, nullptr, 2);

            return Tok(Tok::integer, val);
        }

        Tok parse_oct() {
            auto oct = collect([](char c) {
                if ('0' <= c && c >= '7')
                    return CollectResult::keep;
                else if (c == '_')
                    return CollectResult::skip;
                else
                    return CollectResult::end;
            });

            auto val = std::stoull(oct, nullptr, 8);

            return Tok(Tok::integer, val);
        }

        Tok number0() {
            if (consume('x')) {
                return parse_hex();
            } else if (consume('b')) {
                return parse_bin();
            } else if (consume('o')) {
                return parse_oct();
            } else {
                return parse_number('0');
            }
        }

        Tok parse_number(char c) {
            auto num = collect(c, [](char c) {
                if (isdigit(c)) {
                    return CollectResult::keep;
                } else if (c == '_') {
                    return CollectResult::skip;
                } else {
                    return CollectResult::end;
                }
            });

            return Tok(Tok::integer, std::stoull(num));
        }

        Tok number(char c) {
            if (c == '0') {
                return number0();
            } else {
                return parse_number(c);
            }
        }

        Tok symbol(char c) {
            switch (c) {
            case '+': return Tok(Tok::key, consume('=') ? Keyword::ADDEQ : Keyword::ADD);
            case '-': return Tok(Tok::key, consume('=') ? Keyword::SUBEQ : Keyword::SUB);
            case '/': return Tok(Tok::key, consume('=') ? Keyword::DIVEQ : Keyword::DIV);
            case '*': return Tok(Tok::key, consume('=') ? Keyword::MULEQ : Keyword::MUL);
            case '%': return Tok(Tok::key, consume('=') ? Keyword::MODEQ : Keyword::MOD);
            case '^': return Tok(Tok::key, consume('=') ? Keyword::BITXOREQ : Keyword::BITXOR);
            case '&': return Tok(Tok::key, consume('&') ? Keyword::AND : consume('=') ? Keyword::BITANDEQ : Keyword::BITAND);
            case '|': return Tok(Tok::key, consume('|') ? Keyword::OR : consume('=') ? Keyword::BITOREQ : Keyword::BITOR);
            case '[': return Tok(Tok::key, Keyword::LSQUARE);
            case ']': return Tok(Tok::key, Keyword::RSQUARE);
            case '{': return Tok(Tok::key, Keyword::LBRACE);
            case '}': return Tok(Tok::key, Keyword::RBRACE);
            case '(': return Tok(Tok::key, Keyword::LPAREN);
            case ')': return Tok(Tok::key, Keyword::RPAREN);
            case '!': return Tok(Tok::key, consume('=') ? Keyword::NEQ : Keyword::NOT);
            case '=': return Tok(Tok::key, consume('=') ? Keyword::EQ : Keyword::ASSIGN);
            case '@': return Tok(Tok::key, Keyword::AT);
            case ',': return Tok(Tok::key, Keyword::COMMA);
            case '.': return Tok(Tok::key, Keyword::DOT);
            case '?': return Tok(Tok::key, Keyword::QUESTION);
            case '<': return Tok(Tok::key, consume('<') ? consume('=') ? Keyword::SHLEQ : Keyword::SHL : consume('=') ? Keyword::GTE : Keyword::GT);
            case '>': return Tok(Tok::key, consume('>') ? consume('=') ? Keyword::SHREQ : Keyword::SHR : consume('=') ? Keyword::LTE : Keyword::LT);
            case ':': return Tok(Tok::key, consume(':') ? Keyword::COLON2 : Keyword::COLON);
            case ';': return Tok(Tok::key, Keyword::SEMICOLON);
            default:
                // TODO: handle this
                printf("TODO 2 %c\n", c);
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

        std::istream *source;
        int ahead;

        int read() {
            int temp = ahead;
            ahead = source->get();

            pos.dist++;

            if (temp == '\n') {
                pos.col = 0;
                pos.line++;
            } else {
                pos.col++;
            }

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

        SourcePos pos;
    };
}