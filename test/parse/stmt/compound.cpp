#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        { return; }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseStmt(); }, 
        MAKE<Compound>(vec<ptr<Stmt>>({
            MAKE<Return>(nullptr)
        }))
    );

    parse.finish();
}
