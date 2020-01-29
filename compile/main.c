#include "common/common.h"

char fpeekc(FILE* f)
{
    char c = fgetc(f);
    fputc(c, f);
    return c;
}

void fseek_abs(FILE* f, uint64_t pos)
{
    fseek(f, SEEK_SET, pos);
}

int main(int argc, char** argv)
{
    file_t file = {
        fopen(argv[1], "r"),
        fgetc,
        fpeekc,
        fclose,
        fseek_abs,
        ftell
    };

    lexer_t* lex = lexer_alloc(&file);

    // TODO
}