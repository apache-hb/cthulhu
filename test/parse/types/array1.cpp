#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("[char]");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    vec<ptr<NameType>> names = {
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("char")
            )
        )
    };

    parse.expect(
        [&]{ return parse.parseType(); }, 
        MAKE<ArrayType>(MAKE<QualifiedType>(names), nullptr)
    );

    parse.finish();
}
