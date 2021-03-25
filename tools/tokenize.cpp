#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include <cthulhu2/lexer.h>

int main(int argc, const char** argv) {
    std::cout << ">>> " << std::flush;
    std::string name;
    FILE* file;
    
    if (argc > 1) {
        name = argv[1];
        file = fopen(argv[1], "r");
    } else {
        name = "stdin";
        file = stdin;
    }

    auto handle = FileStream(file);
    auto lexer = Lexer(&handle, name);

    while (true) {
        auto token = lexer.read();

        std::cerr << token.pretty() << std::endl;

        if (token.type == Token::END) {
            break;
        }

        std::cout << ">>> " << std::flush;
    }
}
