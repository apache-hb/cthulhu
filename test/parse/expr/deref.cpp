#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("name->field");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<AccessExpr>(
            MAKE<NameExpr>(parse.qualified({ "name" })),
            MAKE<Ident>(lexer.ident("field")),
            true
        )
    );

    parse.finish();
}
