#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("'a' 'b'");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<CharExpr>('a')
    );

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<CharExpr>('b')
    );

    parse.finish();
}
