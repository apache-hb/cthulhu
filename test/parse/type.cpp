#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8R"(
        int!<int>(int) 
        [int:64] 
        *int::int 
        5 + 5 * 5 
        c::a.b.c
        a(50 + 60, a.b.c)
        q.a[56 * 24]

        using a = b;

        let id = 500;
        
        let id2: int = 600;

        let [ id3, id4 ] = a;

        using a = c;

        using a::b::c;
        using a::b(...);
        using a::b(c, d);
    )");
    auto parse = cthulhu::Parser(lex);

    auto* node = parse.type();
    puts(node->repr().c_str());

    auto* node2 = parse.type();
    puts(node2->repr().c_str());

    auto* node3 = parse.type();
    puts(node3->repr().c_str());

    auto* node4 = parse.expr();
    puts(node4->repr().c_str());

    auto* node5 = parse.expr();
    puts(node5->repr().c_str());

    auto* node6 = parse.expr();
    puts(node6->repr().c_str());

    auto* node7 = parse.expr();
    puts(node7->repr().c_str());

    auto* node8 = parse.decl();
    puts(node8->repr().c_str());

    auto* node9 = parse.decl();
    puts(node9->repr().c_str());

    auto* node10 = parse.decl();
    puts(node10->repr().c_str());

    auto* node11 = parse.decl();
    puts(node11->repr().c_str());

    auto* node12 = parse.include();
    puts(node12->repr().c_str());

    auto* node13 = parse.include();
    puts(node13->repr().c_str());

    auto* node14 = parse.include();
    puts(node14->repr().c_str());

    auto* node15 = parse.include();
    puts(node15->repr().c_str());
}
