#include "lex.h"
#include "ast.h"
#include "parse.h"

int main(int argc, char** argv)
{
    Lexer lex(fopen(argv[argc-1], "r"));

    for(int i = 0; i < 5; i++) {
        auto tok = lex.next();
        printf("%s\n", tok.str());
    }
}