#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("(int, int) -> void");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    vec<ptr<NameType>> names = {
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("void")
            )
        )
    };

    vec<ptr<NameType>> ints = {
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("int")
            )
        )
    };

    parse.expect(
        [&]{ return parse.parseType(); }, 
        MAKE<ClosureType>(vec<ptr<Type>>(
            { MAKE<QualifiedType>(ints), MAKE<QualifiedType>(ints) }
        ), MAKE<QualifiedType>(names))
    );

    parse.finish();
}
