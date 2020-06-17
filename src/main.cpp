#include "parser.cpp"

#include <fstream>

int main(int argc, const char **argv) {
    (void)argc;
    using iflexer = ct::Lexer<std::ifstream>;
    ct::Parser<iflexer> parser(argv[1]);

    auto prog = parser.parse_program();

    for (auto each : prog.imports) {
        printf("import ");
        for (auto part : each.path) {
            printf("%s ", part.c_str());
        }
        printf("(");
        if (each.items.empty()) {
            printf("*");
        } else {
            for (auto part : each.items) {
                printf("%s ", part.c_str());
            }
        }
        printf(");\n");
    }
}