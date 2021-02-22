#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8" \"some \\\" string\" ");

    lex->expect<cthulhu::String>(u8"some \" string");
    lex->expect<cthulhu::End>();
}
