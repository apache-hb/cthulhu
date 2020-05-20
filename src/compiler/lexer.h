#include "stream.h"

#include <variant>
#include <map>

enum class Key {
    DEF,
    TYPE,
    INCLUDE,
    CONST,
    STRUCT,
    UNION,
    ENUM,
    ANY,

    SEMICOLON,
    COLON,
    COLON2,
    ARROW,
    BIGARROW,
    COMMA,
    ASSIGN,

    EQ,
    NEQ,

    ADD,
    ADDEQ,

    SUB,
    SUBEQ,

    DIV,
    DIVEQ,

    MUL,
    MULEQ,

    MOD,
    MODEQ
};

struct Token {
    enum {
        IDENT,
        KEYWORD,
        END,
        ERROR
    } type;

    std::variant<
        std::string,
        Key,
        uint64_t
    > data;
};

const std::map<std::string, Key> keywords = {
    { "def", Key::DEF },
    { "type", Key::TYPE },
    { "include", Key::INCLUDE },
    { "struct", Key::STRUCT },
    { "union", Key::UNION },
    { "enum", Key::ENUM },
    { "any", Key::ANY },
    { "const", Key::CONST }
};

struct Lexer {
    Lexer(Stream i)
        : in(i)
    { }

    Token next() {
        char c = skip_whitespace();

        while(c == '/') {
            if(in.peek() == '/') {
                while((c = in.next()) != '\n');
            } else if(in.peek() == '*') {
                int depth = 1;
                while(depth) {
                    c = in.next();
                    if(c == '/' && in.peek() == '*') {
                        c = in.next();
                        depth++;
                    } else if(c == '*' && in.peek() == '/') {
                        c = in.next();
                        depth--;
                    }
                }
            } else if(in.peek() == '=') {
                return Token{Token::KEYWORD, Key::DIVEQ};
            } else {
                return Token{Token::KEYWORD, Key::DIV};
            }
            c = skip_whitespace();
        }

        if(isalpha(c) || c == '_') {
            std::string buf = {c};
            while(isalnum(in.peek()) || in.peek() == '_') {
                buf += in.next();
            }

            if(auto key = keywords.find(buf); key != keywords.end()) {
                return Token{Token::KEYWORD, key->second};
            } else {
                return Token{Token::IDENT, buf};
            }
        } else {
            switch(c) {
            case ';': return Token{Token::KEYWORD, Key::SEMICOLON};
            }
        }
    }

    char skip_whitespace() {
        char c = in.next();
        while(isspace(c)) {
            c = in.next();
        }
        return c;
    }

    Stream in;
};