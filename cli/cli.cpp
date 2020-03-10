#include <iostream>

#include <ctu/ctu.h>

int main(int argc, const char** argv)
{
    std::cout << ">> ";
    ctu::Lexer lex = { &std::cin };
    ctu::Parser parse = { &lex };

    for(;;)
    {
        std::cout << lex.peek()->to_string() << std::endl;
        std::cout << ">> ";
        lex.next();
    }
}