#pragma once

#include <string>

struct FilePos {
    int col;
    int line;
    int dist;
};

struct Token {
    enum {
        IDENT,
        KEY,
        END,
        INVALID
    } type;

    virtual const char* str() const { 
        switch(type) {
        case IDENT: return "Ident";
        case KEY: return "Key";
        case END: return "End";
        case INVALID: return "Invalid";
        }

        return "Unknown";
    }

    FilePos pos;

    virtual ~Token() {}
    Token(decltype(type) t)
        : type(t)
    { }
};

struct Ident : Token {
    std::string ident;
    Ident(std::string i)
        : Token(Token::IDENT)
        , ident(i)
    { }
};

struct Key : Token {
    enum {
        COLON,

        LBRACE,
        RBRACE,

        LPAREN,
        RPAREN,

        LSQUARE,
        RSQUARE,

        NEWLINE, // can be ignored by the parser
        SEMICOLON // cannot be ignored by the parser
    } key;

    Key(decltype(key) k) 
        : Token(Token::KEY)
        , key(k)
    { }
};

struct End : Token {
    End() : Token(Token::END) { }
};

struct Invalid : Token {
    std::string reason;
    Invalid(std::string r)
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
        auto c = skip_whitespace();

        while(c == '#') {
            c = skip_comment();
        }

        here = pos;

        if(c == EOF) {
            return End();
        } else if(isalpha(c) || c == '_') {
            
        }

        return Invalid("hmm yes");
    }

    char skip_comment() {
        auto c = get();
        while(c != '\n')
            c = get();

        c = skip_whitespace();
        return c;
    }

    char skip_whitespace() {
        auto c = get();
        while(isspace(c))
            c = get();

        return c;
    }

    char get() {
        auto c = fgetc(file);

        pos.dist++;
        if(c == '\n') {
            pos.col = 0;
            pos.line++;
        } else {
            pos.col++;
        }

        return c;
    }

    FilePos here = { 0, 0, 0 };
    FilePos pos = { 0, 0, 0 };
    FILE* file;
};