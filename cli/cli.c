#include <ctu/ctu.h>

#include <stdio.h>

int ctu_next(void* handle)
{
    return fgetc((FILE*)handle);
}

int main(int argc, const char** argv)
{
    ctu_file file = {
        .handle = fopen(argv[1], "r"),
        .next = ctu_next
    };

    ctu_lexer lex = ctu_lexer_new(file);

    ctu_token tok = ctu_lexer_next(&lex);
    printf("%d\n", tok.type);

    return 0;
}