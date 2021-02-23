#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8"int!<int>(int) int[]");
    auto parse = cthulhu::Parser(lex);

    auto* node = parse.type();
    puts(node->repr().c_str());

    auto* node2 = parse.type();
    puts(node2->repr().c_str());
}
