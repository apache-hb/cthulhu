#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        @name(argument)
        using num = int;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Decorated>(
            vec<ptr<Attribute>>({
                MAKE<Attribute>(
                    parse.qualified({ "name" }),
                    vec<ptr<CallArg>>({
                        MAKE<CallArg>(
                            nullptr,
                            MAKE<NameExpr>(parse.qualified({ "argument" }))
                        )
                    })
                )
            }),
            MAKE<Alias>(
                MAKE<Ident>(lexer.ident("num")),
                parse.qualified({ "int" })
            )
        )
    );

    parse.finish();
}
