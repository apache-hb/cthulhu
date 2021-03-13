#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        a = 10;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseStmt(); }, 
        MAKE<Assign>(Assign::ASSIGN,
            MAKE<NameExpr>(parse.qualified({ "a" })),
            MAKE<IntExpr>(Number(10, nullptr))
        )
    );

    parse.finish();
}
