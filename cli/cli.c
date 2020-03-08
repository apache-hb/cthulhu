#include <ctulang/ctu.h>

#include <stdio.h>

typedef struct {
    const char* buf;
    size_t pos;
} ctu_sstream;

int ssnext(void* h)
{
    ctu_sstream* s = (ctu_sstream*)h;

    if(s->pos > strlen(s->buf))
        return EOF;

    return s->buf[s->pos++];
}

int sspeek(void* h)
{
    ctu_sstream* s = (ctu_sstream*)h;

    return s->buf[s->pos];
}

void ssseek(void* h, size_t pos)
{
    ctu_sstream* s = (ctu_sstream*)h;

    s->pos = pos;
}

int main(int argc, const char** argv)
{
    ctu_sstream stream;
    stream.buf = "def main(argc: int, argv: [[char]]) -> int {}";
    stream.pos = 0;

    ctu_input input;
    input.handle = &stream;
    input.next = ssnext;
    input.peek = sspeek;
    input.seek = ssseek;

    ctu_lexer lex = ctu_new_lexer(input);

    int i = 0;

    while(i < 20)
    {
        ctu_token tok = ctu_lexer_next(&lex);
        ctu_print_token(tok);
        ctu_free_token(tok);
        i++;
    }

    return 0;
}