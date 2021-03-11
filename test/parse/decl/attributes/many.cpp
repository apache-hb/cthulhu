#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        @[first, second]
        using num = int;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Decorated>(
            vec<ptr<Attribute>>({
                MAKE<Attribute>(
                    parse.qualified({ "first" }),
                    vec<ptr<CallArg>>()
                ),
                MAKE<Attribute>(
                    parse.qualified({ "second" }),
                    vec<ptr<CallArg>>()
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
