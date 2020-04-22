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

#include "args.h"

typedef struct {
    vec_str_struct imports;
    vec_str_struct aliases;

    vec_str_struct types;
} OutputContext;

void FormatNode(OutputContext* ctx, Node* node)
{
    (void)ctx;
    (void)node;
}

int main(int argc, const char** argv)
{
    Lexer lex;
    Parser parse;
    OutputContext ctx;
    ArgData args;

    args = ArgParse(argc, argv);

    if(argc > 1)
    {
        lex = NewLexer(fopen(args.sources.arr[0], "r+"));
    }
    else
    {
        lex = NewLexer(stdin);
    }
    
    parse = NewParser(&lex);

    vec_str_init(&ctx.imports);
    vec_str_init(&ctx.aliases);
    vec_str_init(&ctx.types);

    for(;;)
    {
        Node* node = ParserNext(&parse);
        if(!node)
            break;

        FormatNode(&ctx, node);
    }

    return 0;
}
