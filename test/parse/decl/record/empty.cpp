#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        record name {}
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Record>(
            MAKE<Ident>(lexer.ident("name")),
            vec<ptr<Field>>()
        )
    );

    parse.finish();
}
