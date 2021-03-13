#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        def name(arg: type, arg2: type2 = 5): int;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Function>(
            MAKE<Ident>(lexer.ident("name")),
            vec<ptr<Param>>({
                MAKE<Param>(
                    MAKE<Ident>(lexer.ident("arg")),
                    parse.qualified({ "type" }),
                    nullptr
                ),
                MAKE<Param>(
                    MAKE<Ident>(lexer.ident("arg2")),
                    parse.qualified({ "type2" }),
                    MAKE<IntExpr>(Number(5, nullptr))
                )
            }),
            parse.qualified({ "int" }),
            nullptr
        )
    );

    parse.finish();
}
