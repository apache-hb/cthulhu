#ifndef CTHULHU_H
#define CTHULHU_H

#include <stddef.h>
#include <string>

typedef enum TokenType {
    INVALID,
    KEYWORD,
    IDENT,
    INT,
    STRING,
    CHAR
} TokenType;

typedef enum Keyword {
    K_INVALID,
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.inc"
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
    ERR_NONE, // no error
    ERR_BAD_ALLOC, // allocation failed
    ERR_BAD_CHAR, // invalid char code
    ERR_INVALID_SYMBOL // invalid symbol, but valid char code
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
    bool eat(char c);

    std::string collect(char c, bool(*func)(char));

    Token ident(char c);
    Keyword symbol(char c);
    Token digit(char c);

    Stream *source;
    SourceRange here;
};

struct Node {

};

struct Type : Node {

};

struct PtrType : Type {

};

struct NameType : Type {

};

struct QualType : Type {

};

struct Parser {

    Node* parse() {
        return nullptr;
    }

    Node* type() {

    }

    Token next() {

    }

    Token peek() {
        
    }

    Token tok = { INVALID };
    Lexer *source;
};

#endif /* CTHULHU_H */
