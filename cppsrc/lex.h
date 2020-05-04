#pragma once

#include <string>

struct FilePos {
    int col;
    int line;
    int dist;
};

struct Token {
    enum {
        IDENT = 0,
        KEY = 1,
        END = 2,
        INVALID = 3
    } type;

    using type_t = decltype(type);

    virtual const char* str() const { 
        switch(type) {
        case IDENT: return "Ident";
        case KEY: return "Key";
        case END: return "End";
        case INVALID: return "Invalid";
        }

        return "Unknown";
    }

    template<typename T>
    bool is() const { printf("t %d", this->type); return this->type == T::self; }

    template<typename T>
    T* as() { 
        if(is<T>()) {
            return static_cast<T*>(this);
        } else {
            return nullptr;
        }
    }

    FilePos pos;

    virtual ~Token() {}
    Token(decltype(type) t)
        : type(t)
    { }
};

struct Ident : Token {
    static constexpr Token::type_t self = Token::IDENT;

    Ident() : Token(Token::IDENT) { }

    std::string ident;
    Ident(std::string i)
        : Token(Token::IDENT)
        , ident(i)
    { }
};

struct Key : Token {
    enum {
        TYPE,
        DEF,
        IMPORT,

        ASSIGN,
        COLON,
        EQ,
        NEQ,
        NOT,

        SUB,

        ARROW,

        LBRACE,
        RBRACE,

        LPAREN,
        RPAREN,

        LSQUARE,
        RSQUARE,

        NEWLINE, // can be ignored by the parser
        SEMICOLON // cannot be ignored by the parser
    } key;

    static constexpr Token::type_t self = Token::KEY;

    using key_t = decltype(key);

    Key() : Token(Token::KEY) {}

    Key(key_t k) 
        : Token(Token::KEY)
        , key(k)
    { }
};

struct End : Token {
    static constexpr Token::type_t self = Token::END;
    End() : Token(Token::END) { }
};

struct Invalid : Token {
    static constexpr Token::type_t self = Token::INVALID;

    std::string reason;
    Invalid(std::string r = "internal parser error")
        : Token(Token::INVALID)
        , reason(r)
    { }
};

struct Lexer {
    Lexer(FILE* f)
        : file(f)
    {}

    Token next() {
        auto tok = parse();
        tok.pos = here;
        return tok;
    }

    Token parse() {
        auto c = get();
        while(isspace(c) || c == '#') {
            if(c == '\n') {
                return Key(Key::NEWLINE);
            } else if(c == '#') {
                while(c != '\n') {
                    c = get();
                }
                return Key(Key::NEWLINE);
            }

            c = get();
        }

        
        if(c == EOF) {
            return End();
        } else if(isalpha(c) || c == '_') {
            std::string buf = {c};
            while(isalnum(peek()) || peek() == '_')
                buf += get();

            if(buf == "type") {
                return Key(Key::TYPE);
            } else if(buf == "def") {
                return Key(Key::DEF);
            } else if(buf == "import") {
                return Key(Key::IMPORT);
            } else {
                return Ident(buf);
            }
        } else if(isdigit(c)) {
            // numbers
        } else {
            switch(c) {
            case ':': return consume('=') ? Key(Key::ASSIGN) : Key(Key::COLON);
            case ';': return Key(Key::SEMICOLON);
            case '=': if(consume('=')) return Key(Key::EQ); else return Invalid("assign with := not =");
            case '!': return consume('=') ? Key(Key::NEQ) : Key(Key::NOT);
            case '-': return consume('>') ? Key(Key::ARROW) : Key(Key::SUB);
            default: return Invalid("invalid symbol");
            }
        }

        return Invalid("cant get here");
    }

    char get() {
        auto c = ahead;
        ahead = fgetc(file);

        pos.dist++;
        if(c == '\n') {
            pos.col = 0;
            pos.line++;
        } else {
            pos.col++;
        }

        return c;
    }

    char peek() {
        return ahead;
    }

    bool consume(char c) {
        if(peek() == c) {
            get();
            return true;
        }
        return false;
    }

    char ahead = ' ';

    FilePos here = { 0, 0, 0 };
    FilePos pos = { 0, 0, 0 };
    FILE* file;
};