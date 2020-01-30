#include "common/common.h"

char next_file(void* f)
{
    return fgetc(f);    
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

    token_t tok;
    
    tok = lexer_next(lex);
    printf("1 = %d\n", tok.type);

    tok = lexer_next(lex);
    printf("2 = %d\n", tok.type);
}