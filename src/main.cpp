#include "parser.h"

#include <stdio.h>

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;
    stream_t in = stream_fopen(argv[1]);
    lexer_t lex = lexer_new(in);
    parser_t parser = parser_new(lex);
    ast_t ast = produce_ast(&parser);
}