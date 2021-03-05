#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("121004");
    auto lexer = TestLexer(&stream);

    lexer.expectNumber(121004);
    lexer.expect(Token::END);
}
