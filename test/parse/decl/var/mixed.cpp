#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        var [lhs: int, rhs] = expr();
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Var>(
            vec<ptr<VarName>>({
                MAKE<VarName>(
                    MAKE<Ident>(lexer.ident("lhs")),
                    parse.qualified({ "int" })
                ),
                MAKE<VarName>(
                    MAKE<Ident>(lexer.ident("rhs")),
                    nullptr
                )
            }),
            MAKE<CallExpr>(
                MAKE<NameExpr>(parse.qualified({ "expr" })),
                vec<ptr<CallArg>>()
            ),
            true
        )
    );

    parse.finish();
}
