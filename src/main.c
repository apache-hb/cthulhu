#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

char* strdup(const char* str)
{
    int n = strlen(str) + 1;
    char* out = malloc(n);
    if(out)
        memcpy(out, str, n);
    
    return out;
}

#include "lex.h"
#include "parse.h"

int main(int argc, const char** argv)
{
    Lexer lex = NewLexer(stdin);
    Parser parse = NewParser(&lex);

    for(;;)
    {
        printf(">>> ");
        Token tok = LexerNext(&lex);
        PrintToken(tok, stdout);
        TokenFree(tok);
    }

    return 0;
}