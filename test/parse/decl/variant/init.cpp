#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        variant name : int {
            case first = 10;
            case second = 25;
            case third = 50;
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
                    MAKE<Ident>(lexer.ident("first")), 
                    MAKE<IntExpr>(Number(10, nullptr)),
                    vec<ptr<Field>>()
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("second")), 
                    MAKE<IntExpr>(Number(25, nullptr)),
                    vec<ptr<Field>>()
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("third")), 
                    MAKE<IntExpr>(Number(50, nullptr)),
                    vec<ptr<Field>>()
                )
            })
        )
    );

    parse.finish();
}
