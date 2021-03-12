#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("name()");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<CallExpr>(
            MAKE<NameExpr>(
                parse.qualified({ "name" })
            ),
            vec<ptr<CallArg>>()
        )
    );

    parse.finish();
}
