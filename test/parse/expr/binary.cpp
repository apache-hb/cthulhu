#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("1 + 2 * 5 + 10");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<BinaryExpr>(
            BinaryExpr::ADD,
            MAKE<BinaryExpr>(
                BinaryExpr::ADD,
                MAKE<IntExpr>(Number(1, nullptr)),
                MAKE<BinaryExpr>(
                    BinaryExpr::MUL,
                    MAKE<IntExpr>(Number(2, nullptr)),
                    MAKE<IntExpr>(Number(5, nullptr))
                )
            ),
            MAKE<IntExpr>(Number(10, nullptr))
        )
    );

    parse.finish();
}
