#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("0x1afb5u32 0b1100011ll 12345f 555w");
    auto lexer = TestLexer(&stream);

    lexer.expectNumber(0x1afb5, "u32");
    lexer.expectNumber(0b1100011, "ll");
    lexer.expectNumber(12345, "f");
    lexer.expectNumber(555, "w");
    lexer.expect(Token::END);
}
