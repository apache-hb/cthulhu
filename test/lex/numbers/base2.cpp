#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("0b1100");
    auto lexer = TestLexer(&stream);

    lexer.expectNumber(0b1100);
    lexer.expect(Token::END);
}
