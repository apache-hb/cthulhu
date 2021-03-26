#pragma once

#include "lexer.h"

struct Parser {
    Parser(Lexer* source);
    
    Token next();
    Token peek();

    Token ahead;
    Lexer* source;
};