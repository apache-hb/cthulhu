#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8"25 0x100 0b1100 25u");

    lex->expect<cthulhu::Int>(25);
    lex->expect<cthulhu::Int>(0x100);
    lex->expect<cthulhu::Int>(0b1100);
    lex->expect<cthulhu::Int>(25, u8"u");
    lex->expect<cthulhu::End>();
}
