#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>

typedef enum TokenType {
    INVALID,
    KEYWORD,
    IDENT,
    INT,
    STRING,
    CHAR
} TokenType;

typedef enum Keyword {
    K_INVALID
} Keyword;

typedef union TokenData {
    char* str;
    char* ident;
    size_t num;
    size_t c;
    Keyword key;
} TokenData;

typedef struct SourceRange {
    size_t offset;
    size_t length;
    size_t line;
    size_t column;
} SourceRange;

typedef struct Token {
    TokenType type;
    TokenData data;
    SourceRange where;
} Token;

typedef enum Error {
    ERR_NONE,
    ERR_BAD_ALLOC,
    ERR_BAD_CHAR
} Error;

struct Stream {
    Stream(void *handle, int(*get)(void*));

    char peek();
    char next();
    bool eat(char c);

private:
    void *data;
    int(*read)(void*);
    int ahead;
};

struct Lexer {
    Lexer(Stream *in);

    Token lex();

private:
    char skip();
    char next();
    char peek();

    Token ident(char c);
    Token symbol(char c);

    Stream *source;
    SourceRange here;
};

#endif /* CTHULHU_H */
