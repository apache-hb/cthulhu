#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        def name(arg: type): int;
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
                )
            }),
            parse.qualified({ "int" }),
            nullptr
        )
    );

    parse.finish();
}
