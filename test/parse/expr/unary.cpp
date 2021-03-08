#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("!true");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<UnaryExpr>(UnaryExpr::NOT, MAKE<BoolExpr>(true))
    );

    parse.finish();
}
