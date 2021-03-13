#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        if true { 

        } else if false { 

        }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseStmt(); }, 
        MAKE<Branch>(vec<ptr<If>>({
            MAKE<If>(MAKE<BoolExpr>(true), MAKE<Compound>(vec<ptr<Stmt>>())),
            MAKE<If>(MAKE<BoolExpr>(false), MAKE<Compound>(vec<ptr<Stmt>>()))
        }))
    );

    parse.finish();
}
