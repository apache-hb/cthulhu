#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("0x1afb5 0b1100011 12345 555");
    auto lexer = TestLexer(&stream);

    lexer.expectNumber(0x1afb5);
    lexer.expectNumber(0b1100011);
    lexer.expectNumber(12345);
    lexer.expectNumber(555);
    lexer.expect(Token::END);
}
