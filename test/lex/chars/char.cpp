#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"('a' '\n' '\x5f\x50')");
    auto lexer = TestLexer(&stream);

    lexer.expectChar('a');
    lexer.expectChar('\n');
    lexer.expectChar(0x5f50);
    
    lexer.expect(Token::END);
}
