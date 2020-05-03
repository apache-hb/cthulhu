#include "lex.h"
#include "ast.h"
#include "parse.h"

int main(int argc, char** argv)
{
    Lexer lex(fopen(argv[1], "r"));
}