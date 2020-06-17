#include "lexer.cpp"

#include <fstream>

int main(int argc, const char **argv) {
    (void)argc;
    ct::Lexer<std::ifstream> lex(argv[1]);

    ct::Token tok = lex.next();
    while (!tok.is(ct::Token::eof)) {
        printf ("%s\n", tok.str().c_str());
        tok = lex.next();
    }
}