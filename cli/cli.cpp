#include <iostream>

#include <ctu/ctu.h>

struct TokenVisitor
{
    std::string operator()(ctu::Key k) const
    {
#define ASM_KEYWORD(id, str) case ctu::Keyword::id: return "Key(" str ")";
#define OPERATOR(id, str) case ctu::Keyword::id: return "Key(" str ")";
#define KEYWORD(id, str) case ctu::Keyword::id: return "Key(" str ")";
#define RES_KEYWORD(id, str) case ctu::Keyword::id: return "Key(" str ")";

        switch(k.key)
        {
#include "ctu/keywords.inc"
        default: return "Key(invalid)";
        }

    }

    std::string operator()(const ctu::Ident& i) const { return "Ident(" + i.ident + ")"; }

    std::string operator()(ctu::Int i) const { return "Int(" + std::to_string(i.num) + ")"; }

    std::string operator()(ctu::Float f) const { return "Float(" + std::to_string(f.num) + ")"; }

    std::string operator()(const ctu::String& s) const { return "String(\"" + s.str + "\")"; }

    std::string operator()(ctu::Char c) const { return "Char(" + std::to_string(c.c) + ")"; }

    std::string operator()(const ctu::Invalid& i) const { return "Invalid(" + i.reason + ")"; }
};

int main(int argc, const char** argv)
{
    std::cout << ">> ";
    ctu::Lexer lex = { &std::cin };

    for(;;)
    {
        std::cout << std::visit(TokenVisitor(), lex.peek()) << std::endl;
        std::cout << ">> ";
        lex.next();
    }
}