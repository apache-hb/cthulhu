#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        while true { }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseStmt(); }, 
        MAKE<While>(
            MAKE<BoolExpr>(true),
            MAKE<Compound>(vec<ptr<Stmt>>()),
            nullptr
        )
    );

    parse.finish();
}
