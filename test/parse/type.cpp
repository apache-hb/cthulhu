#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8"int!<int>(int)");
    auto parse = cthulhu::Parser(lex);

    auto* node = parse.type();
    puts(node->repr().c_str());
}
