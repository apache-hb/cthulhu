#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        return 10;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseStmt(); }, 
        MAKE<Return>(MAKE<IntExpr>(Number(10, nullptr)))
    );

    parse.finish();
}
