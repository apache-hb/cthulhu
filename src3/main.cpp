#include "lexer.cpp"

#include <fstream>

int main(int argc, const char **argv) {
    (void)argc;
    (void)argv;
    ct::Lexer lex(new std::ifstream(argv[1]), argv[1]);

    while (true) {
        auto tok = lex.next();
        if (tok.type == ct::Token::eof)
            break;
        printf("%s", tok.underline("haha yes " + tok.str()).c_str());
    }
}