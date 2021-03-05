#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("0x1afb5");
    auto lexer = TestLexer(&stream);

    lexer.expectNumber(0x1afb5);
    lexer.expect(Token::END);
}
