#include "common/common.h"

int main(int argc, char** argv) {}

#if 0

char next_file(void* f)
{
    char c = fgetc(f);    
    return c;
}

char peek_file(void* f)
{
    char c = fgetc(f);
    ungetc(c, f);
    return c;
}

void seek_file(void* f, uint64_t pos)
{
    fseek(f, SEEK_SET, pos);
}

void close_file(void* f)
{
    fclose(f);
}

uint64_t tell_file(void* f)
{
    return ftell(f);
}

int main(int argc, char** argv)
{
    file_t file = {
        .data = (void*)fopen(argv[1], "r"),
        .next = next_file,
        .peek = peek_file,
        .close = close_file,
        .seek = seek_file,
        .tell = tell_file
    };

    lexer_t* lex = lexer_alloc(&file);

    parser_t* parse = parser_alloc(lex);

    node_t* ast = parser_generate_ast(parse);
}

#endif