#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8"int!<int>(int) [int] *int::int 5 + 5 * 5");
    auto parse = cthulhu::Parser(lex);

    auto* node = parse.type();
    puts(node->repr().c_str());

    auto* node2 = parse.type();
    puts(node2->repr().c_str());

    auto* node3 = parse.type();
    puts(node3->repr().c_str());

    auto* node4 = parse.expr();
    puts(node4->repr().c_str());
}
