#include "lexer.cpp"

#include "ast.h"

#include <fstream>

int main(int argc, const char **argv) {
    (void)argc;
    (void)argv;
    ct::Lexer lex(new std::ifstream(argv[1]), argv[1]);

    while (true) {
        auto tok = lex.next();
        printf("%s", tok.underline("haha yes " + tok.str()).c_str());
        if (tok.type == ct::Token::eof)
            break;
    }
}