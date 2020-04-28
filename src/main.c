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

#if 0
void FormatFuncDecl(Node* node)
{
    printf("func %s\n", node->data.funcDecl.name);
}

void FormatTypeDef(Node* node)
{
    printf("type %s\n", node->data.typeDef.name);
}

void FormatImportDecl(Node* node)
{
    printf("import %s\n", node->data.importDecl.alias);
}

void FormatNode(Node* node)
{
    switch(node->type)
    {
    case NodeTypeFuncDecl:
        FormatFuncDecl(node);
        break;
    case NodeTypeTypeDef:
        FormatTypeDef(node);
        break;
    case NodeTypeImportDecl:
        FormatImportDecl(node);
        break;
    default:
        break;
    }
}
#endif

int main(int argc, const char** argv)
{
    Lexer lex;
    Parser parse;
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

    (void)parse;
    (void)lex;

    parse = NewParser(&lex);

    for(;;)
    {
        Node* node = ParserNext(&parse);
        printf("%p - ", (void*)node);
        if(!node)
            break;

        /* FormatNode(node); */
        /* TODO: properly free node */
        free(node);
    }

    return 0;
}
