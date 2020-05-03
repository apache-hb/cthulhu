#pragma once

#include <string>

struct FilePos {
    int col;
    int line;
    int dist;
};

struct Token {
    FilePos pos;
};

struct Ident : Token {
    std::string ident;
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
};

struct End : Token {};
struct Invalid : Token {};

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
        here = pos;
    }

    FilePos here = { 0, 0, 0 };
    FilePos pos = { 0, 0, 0 };
    FILE* file;
};