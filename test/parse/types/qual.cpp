#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("name::name2");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    vec<ptr<NameType>> names = {
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("name")
            )
        ),
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("name2")
            )
        )
    };

    parse.expect(
        [&]{ return parse.parseType(); }, 
        MAKE<QualifiedType>(names)
    );

    parse.finish();
}
