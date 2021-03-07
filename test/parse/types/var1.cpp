#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("var name");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    vec<ptr<NameType>> names = {
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("name")
            )
        )
    };

    parse.expect(
        [&]{ return parse.parseType(); }, 
        MAKE<MutableType>(MAKE<QualifiedType>(names))
    );

    parse.finish();
}
