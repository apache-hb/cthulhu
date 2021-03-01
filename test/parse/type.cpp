#include "test.hpp"

int main() {
    /*
    auto* lex = cthulhu::string(u8R"(
        int!<int>(int) 
        [int:64] 
        *int::int 
        5 + 5 * 5 
        c::a.b.c
        a(.x = 50 + 60, a.b.c)
        q.a[56 * 24]

        using a = b;

        let id = 500;
        
        var id2: int = 600;

        let [ id3, id4 ] = a;

        using a = c;

        using a::b::c;
        using a::b(...);
        using a::b(c, d);

        using a::b::c;
        using a::b(...);
        using c::d::e(f, g, h);

        using x = f;
    )");
    auto parse = cthulhu::Parser(lex);

    parse.type();
    parse.type();
    parse.type();
    parse.expr();
    parse.expr();
    parse.expr();
    parse.expr();
    parse.decl();
    parse.decl();
    parse.decl();
    parse.decl();
    parse.include();
    parse.include();
    parse.include();
    parse.include();

    cthulhu::Printer printer;

    auto* unit = parse.parseUnit();
    unit->visit(&printer);

    puts(printer.buffer.c_str());*/
}
