#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        using num = int;
        
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Alias>(
            MAKE<Ident>(lexer.ident("num")),
            parse.qualified({ "int" })
        )
    );

    parse.finish();
}
