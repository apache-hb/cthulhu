#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        variant name {
            
        }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Variant>(
            MAKE<Ident>(lexer.ident("name")),
            nullptr,
            vec<ptr<Case>>()
        )
    );

    parse.finish();
}
