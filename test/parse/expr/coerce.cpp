#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("coerce!<int>(100000000ull)");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<CoerceExpr>(
            parse.qualified({ "int" }),
            MAKE<IntExpr>(Number(100000000, lexer.ident("ull").ident()))
        )
    );

    parse.finish();
}
