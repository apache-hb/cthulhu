#include "parser.h"

int main(int argc, char **argv)
{
    (void)argc;
    ct::Lexer lex(fopen(argv[1], "rt"));

    ct::Parser parser{lex};

    auto thing = parser.parse();
    (void)thing;

    for (auto each : thing.imports)
    {
        printf("import ");
        int i = 0;
        for (auto part : each.path)
        {
            if (i++ != 0)
                printf("::");
            printf("%s", part.c_str());
        }
    }
}