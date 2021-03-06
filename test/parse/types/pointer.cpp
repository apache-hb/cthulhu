#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("*name::name2 **hello");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    vec<ptr<NameType>> names = {
        MAKE<NameType>(
            MAKE<Ident>(
                Token(Token::IDENT, { .ident = lexer.idents.intern("name") })
            )
        ),
        MAKE<NameType>(
            MAKE<Ident>(
                Token(Token::IDENT, { .ident = lexer.idents.intern("name2") })
            )
        )
    };

    parse.expect(
        [&]{ return parse.parseType(); }, 
        MAKE<PointerType>(MAKE<QualifiedType>(names))
    );

    vec<ptr<NameType>> second = {
        MAKE<NameType>(
            MAKE<Ident>(
                Token(Token::IDENT, { . ident = lexer.idents.intern("hello") })
            )
        )
    };

    parse.expect(
        [&] { return parse.parseType(); },
        MAKE<PointerType>(MAKE<PointerType>(MAKE<QualifiedType>(second)))
    );

    parse.finish();
}
