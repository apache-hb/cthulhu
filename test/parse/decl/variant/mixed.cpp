#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        variant name : int {
            case first(lhs: int, rhs: int) = 10;
            case second(body: bool);
            case third;
            case fourth = 25;
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
                    vec<ptr<Field>>({
                        MAKE<Field>(
                            MAKE<Ident>(lexer.ident("lhs")),
                            parse.qualified({ "int" })
                        ),
                        MAKE<Field>(
                            MAKE<Ident>(lexer.ident("rhs")),
                            parse.qualified({ "int" })
                        )
                    })
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("second")), 
                    nullptr,
                    vec<ptr<Field>>({
                        MAKE<Field>(
                            MAKE<Ident>(lexer.ident("body")),
                            parse.qualified({ "bool" })
                        )
                    })
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("third")), 
                    nullptr,
                    vec<ptr<Field>>()
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("fourth")), 
                    MAKE<IntExpr>(Number(25, nullptr)),
                    vec<ptr<Field>>()
                )
            })
        )
    );

    parse.finish();
}
