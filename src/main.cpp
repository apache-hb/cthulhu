#include "lexer.h"

int main(int argc, char **argv)
{
    (void)argc;
    ct::Lexer lex(fopen(argv[1], "rt"));

    ct::Token tok;
    do
    {
        tok = lex.next();
        printf("%d\n", tok.type);
    } while (tok.type != ct::Token::END);
}