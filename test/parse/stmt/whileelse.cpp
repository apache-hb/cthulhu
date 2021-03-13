#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        while true { } else { }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseStmt(); }, 
        MAKE<While>(
            MAKE<BoolExpr>(true),
            MAKE<Compound>(vec<ptr<Stmt>>()),
            MAKE<Compound>(vec<ptr<Stmt>>())
        )
    );

    parse.finish();
}
