#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        variant name {
            case first;
            case second;
            case third;
        }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Variant>(
            MAKE<Ident>(lexer.ident("name")),
            nullptr,
            vec<ptr<Case>>({
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("first")), 
                    nullptr,
                    vec<ptr<Field>>()
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("second")), 
                    nullptr,
                    vec<ptr<Field>>()
                ),
                MAKE<Case>(
                    MAKE<Ident>(lexer.ident("third")), 
                    nullptr,
                    vec<ptr<Field>>()
                )
            })
        )
    );

    parse.finish();
}
