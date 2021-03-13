#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        def name;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Function>(
            MAKE<Ident>(lexer.ident("name")),
            vec<ptr<Param>>(),
            nullptr,
            nullptr
        )
    );

    parse.finish();
}
