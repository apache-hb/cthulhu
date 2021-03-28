#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include <cthulhu2/lexer.h>

int main(int argc, const char** argv) {
    std::string name;
    FILE* file;
    Pool pool;
    
    if (argc > 1) {
        name = argv[1];
        file = fopen(argv[1], "r");
    } else {
        name = "stdin";
        file = stdin;
    }

    std::cout << ">>> " << std::flush;

    auto handle = FileStream(file);
    auto lexer = Lexer(&handle, name, &pool);

    while (true) {
        auto token = lexer.read();

        std::cerr << token.pretty() << std::endl;

        if (token.type == Token::END) {
            break;
        }

        std::cout << ">>> " << std::flush;
    }
}
