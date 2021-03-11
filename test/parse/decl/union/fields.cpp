#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        union name {
            name1: type1;
            name2: type2;
        }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Union>(
            MAKE<Ident>(lexer.ident("name")),
            vec<ptr<Field>>({
                MAKE<Field>(
                    MAKE<Ident>(lexer.ident("name1")), 
                    parse.qualified({ "type1" })
                ),
                MAKE<Field>(
                    MAKE<Ident>(lexer.ident("name2")), 
                    parse.qualified({ "type2" })
                )
            })
        )
    );

    parse.finish();
}
