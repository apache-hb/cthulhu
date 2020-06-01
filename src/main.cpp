#include "parser.h"

int main(int argc, char **argv)
{
    (void)argc;
    ct::Lexer lex(fopen(argv[1], "rt"));

    /*auto tok = lex.next();
    while (tok.type != ct::Token::END)
    {
        printf("%s\n", ct::to_string(tok).c_str());
        tok = lex.next();
    }
    std::exit(0);
*/
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
        i = 0;

        printf("(");

        if (each.deps.size() == 0)
            printf("...");
        else
            for (auto part : each.deps)
            {
                if (i++ != 0)
                    printf(", ");
                printf("%s", part.c_str());
            }

        printf(");");

        printf("\n");
    }
}