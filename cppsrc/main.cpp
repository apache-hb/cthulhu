#include "lex.h"
#include "ast.h"
#include "parse.h"

int main(int argc, char** argv)
{
    Lexer lex(fopen(argv[argc-1], "r"));
    Parser parse(lex);

    auto tree = parse.program();

    for(auto path : tree.deps) {
        for(auto part : path.parts) {
            printf("%s:", part.c_str());
        }
    }
}