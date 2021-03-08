#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("55 100u8");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<IntExpr>(Number(55, nullptr))
    );

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<IntExpr>(Number(100, lexer.ident("u8").ident()))
    );

    parse.finish();
}
