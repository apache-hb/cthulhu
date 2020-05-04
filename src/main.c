#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

static char* strdup(const char* str)
{
    size_t n = strlen(str) + 1;
    char* out = malloc(n);
    if(out)
        memcpy(out, str, n);
    
    return out;
}

#include "lex.h"
#include "parse.h"
#include "writer.h"
#include "args.h"

int main(int argc, const char** argv)
{
    Lexer lex;
    Parser parse;
    ArgData args;

    args = ArgParse(argc, argv);

    if(argc > 1)
    {
        printf("source %s\n", args.sources.arr[0]);
        lex = NewLexer(fopen(args.sources.arr[0], "r+"));
    }
    else
    {
        lex = NewLexer(stdin);
    }

    parse = NewParser(&lex);

    Writer w;
    w.i = 0;

    Node* prog = ParseProgram(&parse);
    PrintProgram(&w, prog);

    return 0;
}
