#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        variant name : int {
            case name(name: type) = 10;
        }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Variant>(
            MAKE<Ident>(lexer.ident("name")),
            parse.qualified({ "int" }),
            vec<ptr<Case>>({
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("name")), 
                    MAKE<IntExpr>(Number(10, nullptr)),
                    vec<ptr<Field>>({
                        MAKE<Field>(
                            MAKE<Ident>(lexer.ident("name")),
                            parse.qualified({ "type" })
                        )
                    })
                )
            })
        )
    );

    parse.finish();
}
