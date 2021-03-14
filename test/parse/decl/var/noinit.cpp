#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        var x: int;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Var>(
            vec<ptr<VarName>>({
                MAKE<VarName>(
                    MAKE<Ident>(lexer.ident("x")),
                    parse.qualified({ "int" })
                )
            }),
            nullptr,
            true
        )
    );

    parse.finish();
}