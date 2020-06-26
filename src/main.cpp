#include "lexer.cpp"
#include "gen/gen.cpp"

#include <fstream>

int main(int, const char **argv) {
    ct::Lexer lex(new std::ifstream(argv[1]), argv[1]);
}