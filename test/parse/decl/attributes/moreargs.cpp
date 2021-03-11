#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        @[one(two), three(four, .five = six)]
        using num = int;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Decorated>(
            vec<ptr<Attribute>>({
                MAKE<Attribute>(
                    parse.qualified({ "one" }),
                    vec<ptr<CallArg>>({
                        MAKE<CallArg>(
                            nullptr,
                            MAKE<NameExpr>(parse.qualified({ "two" }))
                        )
                    })
                ),
                MAKE<Attribute>(
                    parse.qualified({ "three" }),
                    vec<ptr<CallArg>>({
                        MAKE<CallArg>(
                            nullptr,
                            MAKE<NameExpr>(parse.qualified({ "four" }))
                        ),
                        MAKE<CallArg>(
                            MAKE<Ident>(lexer.ident("five")),
                            MAKE<NameExpr>(parse.qualified({ "six" }))
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
