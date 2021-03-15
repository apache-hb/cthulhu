#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include <cthulhu2/lexer.h>

int main(int argc, const char** argv) {
    FILE* file = argc > 1 ? fopen(argv[1], "r") : stdin;
    auto handle = FileStream(file);
    auto lexer = Lexer(&handle);

    while (true) {
        auto token = lexer.read();

        std::cerr << "text: " << token.text() << std::endl;
        std::cerr << "repr: " << token.repr() << std::endl;

        if (token.type == Token::END) {
            break;
        }
    }
}
