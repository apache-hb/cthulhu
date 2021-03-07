#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("true false");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<BoolExpr>(true)
    );

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<BoolExpr>(false)
    );

    parse.finish();
}
