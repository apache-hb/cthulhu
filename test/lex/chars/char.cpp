#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"('a' '\n' '\x50\x50')");
    auto lexer = TestLexer(&stream);

    lexer.expectChar('a');
    lexer.expectChar('\n');
    lexer.expectChar(0x5050);
    
    lexer.expect(Token::END);
}
