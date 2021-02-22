#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8"hello world using");

    lex->expect<cthulhu::Ident>(u8"hello");
    lex->expect<cthulhu::Ident>(u8"world");
    lex->expect<cthulhu::Key>(cthulhu::Key::USING);
    lex->expect<cthulhu::End>();
}
