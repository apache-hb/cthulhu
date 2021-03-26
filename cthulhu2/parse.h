#pragma once

#include "lexer.h"

#include <optional>
#include <string>

struct Error {
    enum Type {
        UNEXPECTED_TYPE
    };

    union Data {
        Token::Type type;
    };

    Error(Type type, Token token, Data data = {}) 
        : type(type)
        , token(token)
        , data(data) 
    { }

    Type type;
    Token token;
    Data data;
};

struct Node {
    std::vector<Token> tokens;
};

struct Import : Node {
    std::vector<Ident> path;
    std::optional<std::vector<Ident>> items;
};

struct Unit : Node {
    std::vector<Import*> imports;
};

struct Parser {
    Parser(Lexer* source);
    
    Node* include();
    Unit* unit();


    std::vector<Ident> idents(Key sep);
    bool eat(Key key);
    Token expect(Token::Type type);

    Token next();
    Token peek();

    Token ahead;
    Lexer* source;
};
